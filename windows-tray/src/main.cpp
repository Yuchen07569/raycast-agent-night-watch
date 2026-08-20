#include "windows_platform.hpp"

#include "agent_night_watch/power_session.hpp"

#include <shellapi.h>

#include <array>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <string>

namespace agent_night_watch::windows {
namespace {

constexpr wchar_t kWindowClass[] = L"AgentNightWatchHiddenWindow";
constexpr wchar_t kMutexName[] = L"Local\\AgentNightWatch.Tray.v1";
constexpr UINT kTrayMessage = WM_APP + 1;
constexpr UINT_PTR kStatusTimer = 1;
constexpr UINT_PTR kDisplayOffTimer = 2;
constexpr UINT kCommandToggle = 1001;
constexpr UINT kCommandLockScreen = 1002;
constexpr UINT kCommandStatus = 1003;
constexpr UINT kCommandRestore = 1004;
constexpr UINT kCommandExit = 1005;

enum class RuntimeState {
  Off,
  Active,
  Paused,
  Warning,
};

std::wstring operation_message(const OperationResult& result) {
  return widen(result.message);
}

class TrayApplication {
 public:
  explicit TrayApplication(const HINSTANCE instance)
      : instance_(instance), owner_pid_(GetCurrentProcessId()) {}

  ~TrayApplication() {
    release_system_required(power_request_);
    remove_tray_icon();
    if (mutex_ != nullptr) CloseHandle(mutex_);
  }

  int run() {
    if (!initialize()) return 1;
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
      TranslateMessage(&message);
      DispatchMessageW(&message);
    }
    return static_cast<int>(message.wParam);
  }

 private:
  bool initialize() {
    mutex_ = CreateMutexW(nullptr, FALSE, kMutexName);
    if (mutex_ == nullptr) {
      show_error(L"Agent Night Watch could not create its single-instance lock.");
      return false;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
      MessageBoxW(nullptr, L"Agent Night Watch is already running.",
                  L"Agent Night Watch", MB_OK | MB_ICONINFORMATION);
      return false;
    }
    if (!journal_.ready()) {
      show_error(L"The recovery directory is unavailable:\n\n" +
                 widen(journal_.startup_error()));
      return false;
    }
    instance_id_ = make_guid_string();
    if (instance_id_.empty()) {
      show_error(L"A secure application instance identifier could not be created.");
      return false;
    }
    if (!recover_stale_session()) return false;

    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(window_class);
    window_class.hInstance = instance_;
    window_class.lpfnWndProc = &TrayApplication::window_procedure;
    window_class.lpszClassName = kWindowClass;
    window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    if (RegisterClassExW(&window_class) == 0 &&
        GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
      show_error(L"The hidden tray window could not be registered.");
      return false;
    }
    window_ = CreateWindowExW(0, kWindowClass, L"Agent Night Watch", 0, 0, 0,
                              0, 0, HWND_MESSAGE, nullptr, instance_, this);
    if (window_ == nullptr) {
      show_error(L"The hidden tray window could not be created.");
      return false;
    }
    taskbar_created_message_ = RegisterWindowMessageW(L"TaskbarCreated");
    if (!add_tray_icon()) {
      show_error(L"The coffee cup could not be added to the system tray.");
      return false;
    }
    std::string watchdog_error;
    if (!start_watchdog(owner_pid_, instance_id_, watchdog_error)) {
      show_error(L"The recovery watchdog could not start:\n\n" +
                 widen(watchdog_error));
      return false;
    }
    SetTimer(window_, kStatusTimer, 5000, nullptr);
    refresh_state();
    return true;
  }

  bool recover_stale_session() {
    std::string error;
    const auto record = journal_.load(error);
    if (!record && error.empty()) return true;
    if (!record) {
      runtime_state_ = RuntimeState::Warning;
      status_message_ = "The recovery journal is damaged. No power settings were changed.";
      MessageBoxW(nullptr,
                  L"An invalid Agent Night Watch recovery journal was found. "
                  L"The app will start in warning mode and will not change power settings.",
                  L"Recovery Required", MB_OK | MB_ICONWARNING);
      return true;
    }
    if (process_is_alive(record->owner_pid)) {
      show_error(L"An unfinished session still belongs to a running process. "
                 L"Close that process before starting Agent Night Watch again.");
      return false;
    }
    const int choice = MessageBoxW(
        nullptr,
        L"Agent Night Watch found an unfinished session. Restore its original "
        L"AC power settings now?\n\nNothing will be enabled automatically.",
        L"Restore Previous Session", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON1);
    if (choice != IDYES) {
      runtime_state_ = RuntimeState::Warning;
      status_message_ = "An unfinished session is waiting for manual recovery.";
      return true;
    }
    const OperationResult restored = restore_session(policy_, journal_);
    if (restored.kind == OperationKind::Success ||
        restored.kind == OperationKind::NoSession) {
      return true;
    }
    runtime_state_ = RuntimeState::Warning;
    status_message_ = restored.message;
    MessageBoxW(nullptr, operation_message(restored).c_str(), L"Recovery Failed",
                MB_OK | MB_ICONWARNING);
    return true;
  }

  static LRESULT CALLBACK window_procedure(HWND window, UINT message,
                                            WPARAM w_param, LPARAM l_param) {
    TrayApplication* self = reinterpret_cast<TrayApplication*>(
        GetWindowLongPtrW(window, GWLP_USERDATA));
    if (message == WM_NCCREATE) {
      const auto* create = reinterpret_cast<CREATESTRUCTW*>(l_param);
      self = static_cast<TrayApplication*>(create->lpCreateParams);
      SetWindowLongPtrW(window, GWLP_USERDATA,
                        reinterpret_cast<LONG_PTR>(self));
    }
    if (self != nullptr) return self->handle_message(message, w_param, l_param);
    return DefWindowProcW(window, message, w_param, l_param);
  }

  LRESULT handle_message(const UINT message, const WPARAM w_param,
                         const LPARAM l_param) {
    if (message == taskbar_created_message_ && taskbar_created_message_ != 0) {
      tray_added_ = false;
      add_tray_icon();
      return 0;
    }
    switch (message) {
      case kTrayMessage:
        if (l_param == WM_LBUTTONUP) toggle();
        if (l_param == WM_RBUTTONUP || l_param == WM_CONTEXTMENU) show_menu();
        return 0;
      case WM_COMMAND:
        handle_command(LOWORD(w_param));
        return 0;
      case WM_TIMER:
        if (w_param == kStatusTimer) refresh_state();
        if (w_param == kDisplayOffTimer) {
          KillTimer(window_, kDisplayOffTimer);
          DWORD_PTR ignored{};
          SendMessageTimeoutW(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER,
                              2, SMTO_ABORTIFHUNG, 1000, &ignored);
        }
        return 0;
      case WM_POWERBROADCAST:
        if (w_param == PBT_APMPOWERSTATUSCHANGE) refresh_state();
        return TRUE;
      case WM_QUERYENDSESSION:
        restore_before_exit(false);
        return TRUE;
      case WM_ENDSESSION:
        if (w_param != FALSE) restore_before_exit(false);
        return 0;
      case WM_DESTROY:
        KillTimer(window_, kStatusTimer);
        remove_tray_icon();
        PostQuitMessage(0);
        return 0;
      default:
        return DefWindowProcW(window_, message, w_param, l_param);
    }
  }

  void handle_command(const UINT command) {
    switch (command) {
      case kCommandToggle:
        toggle();
        break;
      case kCommandLockScreen:
        if (runtime_state_ == RuntimeState::Active) {
          lock_and_turn_off_displays(window_);
        } else {
          MessageBoxW(window_,
                      L"Enable Agent Night Watch on AC power before locking and "
                      L"turning off the displays.",
                      L"Agent Night Watch", MB_OK | MB_ICONINFORMATION);
        }
        break;
      case kCommandStatus:
        show_status();
        break;
      case kCommandRestore:
        manual_restore();
        break;
      case kCommandExit:
        if (restore_before_exit(true)) DestroyWindow(window_);
        break;
      default:
        break;
    }
  }

  void toggle() {
    if (runtime_state_ == RuntimeState::Warning) {
      show_status();
      return;
    }
    if (runtime_state_ == RuntimeState::Off) {
      enable();
    } else {
      disable();
    }
  }

  void enable() {
    const std::string session_id = make_guid_string();
    if (session_id.empty()) {
      show_error(L"A secure session identifier could not be created.");
      return;
    }
    const OperationResult result = enable_session(
        policy_, journal_, session_id, instance_id_, owner_pid_);
    if (!result.ok()) {
      show_error(L"Night Watch was not enabled:\n\n" + operation_message(result));
      refresh_state();
      return;
    }
    if (system_is_on_ac()) {
      std::string request_error;
      if (!acquire_system_required(power_request_, request_error)) {
        const OperationResult rollback = restore_session(
            policy_, journal_, instance_id_, owner_pid_);
        show_error(L"Night Watch was rolled back because Windows rejected the "
                   L"system power request:\n\n" +
                   widen(request_error) + L"\n\n" + operation_message(rollback));
      }
    }
    refresh_state();
  }

  void disable() {
    release_system_required(power_request_);
    const OperationResult result = restore_session(
        policy_, journal_, instance_id_, owner_pid_);
    if (result.kind != OperationKind::Success &&
        result.kind != OperationKind::NoSession) {
      show_error(L"Normal AC power settings could not be restored:\n\n" +
                 operation_message(result));
    }
    refresh_state();
  }

  void manual_restore() {
    const int choice = MessageBoxW(
        window_,
        L"Restore the original AC power settings recorded by Agent Night Watch?\n\n"
        L"If another tool changed those settings, nothing will be overwritten.",
        L"Restore Original Settings", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
    if (choice != IDYES) return;
    release_system_required(power_request_);
    const OperationResult result = restore_session(policy_, journal_);
    if (result.kind != OperationKind::Success &&
        result.kind != OperationKind::NoSession) {
      show_error(L"The recorded settings could not be restored:\n\n" +
                 operation_message(result));
    }
    refresh_state();
  }

  bool restore_before_exit(const bool interactive) {
    std::string error;
    const auto record = journal_.load(error);
    if (!record && error.empty()) {
      release_system_required(power_request_);
      return true;
    }
    release_system_required(power_request_);
    const OperationResult restored = restore_session(policy_, journal_);
    if (restored.kind == OperationKind::Success ||
        restored.kind == OperationKind::NoSession) {
      return true;
    }
    if (interactive) {
      show_error(L"Agent Night Watch will remain open because the original AC "
                 L"settings could not be safely restored:\n\n" +
                 operation_message(restored));
    }
    refresh_state();
    return !interactive;
  }

  void refresh_state() {
    const OperationResult status = validate_owned_session(policy_, journal_);
    if (status.kind == OperationKind::NoSession) {
      runtime_state_ = RuntimeState::Off;
      status_message_ = "Off — normal Windows power behavior.";
      release_system_required(power_request_);
    } else if (status.kind == OperationKind::Success && status.session &&
               status.session->instance_id == instance_id_ &&
               status.session->owner_pid == owner_pid_) {
      if (!system_is_on_ac()) {
        runtime_state_ = RuntimeState::Paused;
        status_message_ =
            "Paused on battery — original battery settings remain active.";
        release_system_required(power_request_);
      } else {
        std::string request_error;
        if (acquire_system_required(power_request_, request_error)) {
          runtime_state_ = RuntimeState::Active;
          status_message_ = status.session->lid_present
                                ? "On — AC sleep and AC lid sleep are disabled."
                                : "On — AC automatic sleep is disabled.";
        } else {
          runtime_state_ = RuntimeState::Warning;
          status_message_ = "Windows rejected the system power request: " +
                            request_error;
        }
      }
    } else {
      runtime_state_ = RuntimeState::Warning;
      status_message_ = status.message;
      release_system_required(power_request_);
    }
    update_tray_icon();
  }

  void show_status() {
    std::wstring state;
    switch (runtime_state_) {
      case RuntimeState::Off:
        state = L"Off";
        break;
      case RuntimeState::Active:
        state = L"On (AC power)";
        break;
      case RuntimeState::Paused:
        state = L"Paused (battery power)";
        break;
      case RuntimeState::Warning:
        state = L"Needs attention";
        break;
    }
    const std::wstring detail = L"State: " + state + L"\n\n" +
                                widen(status_message_) +
                                L"\n\nBattery power settings are never modified.";
    MessageBoxW(window_, detail.c_str(), L"Agent Night Watch Status",
                MB_OK | (runtime_state_ == RuntimeState::Warning
                             ? MB_ICONWARNING
                             : MB_ICONINFORMATION));
  }

  void show_menu() {
    const HMENU menu = CreatePopupMenu();
    if (menu == nullptr) return;
    const bool off = runtime_state_ == RuntimeState::Off;
    const bool normal = runtime_state_ != RuntimeState::Warning;
    AppendMenuW(menu, MF_STRING | (normal ? MF_ENABLED : MF_GRAYED),
                kCommandToggle,
                off ? L"Enable Agent Night Watch" : L"Disable Agent Night Watch");
    AppendMenuW(menu,
                MF_STRING |
                    (runtime_state_ == RuntimeState::Active ? MF_ENABLED
                                                            : MF_GRAYED),
                kCommandLockScreen, L"Lock and Turn Off Displays");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kCommandStatus, L"View Status");
    AppendMenuW(menu, MF_STRING, kCommandRestore,
                L"Restore Original Power Settings…");
    AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(menu, MF_STRING, kCommandExit, L"Exit");

    POINT point{};
    GetCursorPos(&point);
    SetForegroundWindow(window_);
    TrackPopupMenu(menu, TPM_RIGHTBUTTON | TPM_BOTTOMALIGN | TPM_LEFTALIGN,
                   point.x, point.y, 0, window_, nullptr);
    PostMessageW(window_, WM_NULL, 0, 0);
    DestroyMenu(menu);
  }

  bool add_tray_icon() {
    tray_data_ = {};
    tray_data_.cbSize = sizeof(tray_data_);
    tray_data_.hWnd = window_;
    tray_data_.uID = 1;
    tray_data_.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
    tray_data_.uCallbackMessage = kTrayMessage;
    tray_icon_ = create_coffee_icon(TrayVisualState::Off);
    tray_data_.hIcon = tray_icon_;
    wcscpy_s(tray_data_.szTip, L"Agent Night Watch: Off");
    tray_added_ = Shell_NotifyIconW(NIM_ADD, &tray_data_) != FALSE;
    return tray_added_;
  }

  void update_tray_icon() {
    if (!tray_added_) return;
    TrayVisualState visual = TrayVisualState::Off;
    std::wstring tooltip = L"Agent Night Watch: Off";
    if (runtime_state_ == RuntimeState::Active) {
      visual = TrayVisualState::Active;
      tooltip = L"Agent Night Watch: On (AC power)";
    } else if (runtime_state_ == RuntimeState::Paused) {
      visual = TrayVisualState::Paused;
      tooltip = L"Agent Night Watch: Paused on battery";
    } else if (runtime_state_ == RuntimeState::Warning) {
      visual = TrayVisualState::Warning;
      tooltip = L"Agent Night Watch: Needs attention";
    }
    const HICON replacement = create_coffee_icon(visual);
    if (replacement == nullptr) return;
    const HICON previous = tray_icon_;
    tray_icon_ = replacement;
    tray_data_.hIcon = replacement;
    wcsncpy_s(tray_data_.szTip, tooltip.c_str(), _TRUNCATE);
    Shell_NotifyIconW(NIM_MODIFY, &tray_data_);
    if (previous != nullptr) DestroyIcon(previous);
  }

  void remove_tray_icon() {
    if (tray_added_) {
      Shell_NotifyIconW(NIM_DELETE, &tray_data_);
      tray_added_ = false;
    }
    if (tray_icon_ != nullptr) {
      DestroyIcon(tray_icon_);
      tray_icon_ = nullptr;
    }
  }

  void show_error(const std::wstring& message) const {
    MessageBoxW(window_, message.c_str(), L"Agent Night Watch",
                MB_OK | MB_ICONERROR);
  }

  HINSTANCE instance_{};
  HWND window_{};
  HANDLE mutex_{};
  HANDLE power_request_{};
  WindowsPowerPolicy policy_;
  WindowsJournalStore journal_;
  std::uint32_t owner_pid_{};
  std::string instance_id_;
  RuntimeState runtime_state_{RuntimeState::Off};
  std::string status_message_{"Off — normal Windows power behavior."};
  NOTIFYICONDATAW tray_data_{};
  HICON tray_icon_{};
  bool tray_added_{};
  UINT taskbar_created_message_{};
};

std::optional<std::uint32_t> parse_process_id(const wchar_t* value) {
  if (value == nullptr || *value == L'\0') return std::nullopt;
  wchar_t* end = nullptr;
  const unsigned long parsed = wcstoul(value, &end, 10);
  if (end == value || *end != L'\0' || parsed <= 1 ||
      parsed > UINT32_MAX) {
    return std::nullopt;
  }
  return static_cast<std::uint32_t>(parsed);
}

}  // namespace
}  // namespace agent_night_watch::windows

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
  using namespace agent_night_watch::windows;
  CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

  int argument_count = 0;
  wchar_t** arguments = CommandLineToArgvW(GetCommandLineW(), &argument_count);
  if (arguments != nullptr && argument_count == 4 &&
      std::wstring(arguments[1]) == L"--watchdog") {
    const auto process_id = parse_process_id(arguments[2]);
    const std::string instance_id = narrow(arguments[3]);
    LocalFree(arguments);
    const int result = process_id && !instance_id.empty()
                           ? run_watchdog(*process_id, instance_id)
                           : 4;
    CoUninitialize();
    return result;
  }
  if (arguments != nullptr) LocalFree(arguments);

  TrayApplication application(instance);
  const int result = application.run();
  CoUninitialize();
  return result;
}
