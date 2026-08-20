#include "windows_platform.hpp"

#include "agent_night_watch/session.hpp"

#include <objbase.h>
#include <powrprof.h>
#include <shlobj.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <filesystem>
#include <limits>
#include <memory>
#include <string_view>
#include <vector>

namespace agent_night_watch::windows {
namespace {

constexpr GUID kSleepSubgroup = {0x238c9fa8,
                                 0x0aad,
                                 0x41ed,
                                 {0x83, 0xf4, 0x97, 0xbe, 0x24, 0x2c, 0x8f,
                                  0x20}};
constexpr GUID kStandbyTimeout = {0x29f6c1db,
                                  0x86da,
                                  0x48c5,
                                  {0x9f, 0xdb, 0xf2, 0xb6, 0x7b, 0x1f, 0x44,
                                   0xda}};
constexpr GUID kSystemButtonSubgroup = {
    0x4f971e89,
    0xeebd,
    0x4455,
    {0xa8, 0xde, 0x9e, 0x59, 0x04, 0x0e, 0x73, 0x47}};
constexpr GUID kLidCloseAction = {0x5ca83367,
                                  0x6e45,
                                  0x459f,
                                  {0xa2, 0x7b, 0x47, 0x6b, 0x1d, 0x01, 0xc9,
                                   0x36}};

const GUID& subgroup_for(const AcSetting setting) {
  return setting == AcSetting::StandbyTimeout ? kSleepSubgroup
                                               : kSystemButtonSubgroup;
}

const GUID& setting_guid_for(const AcSetting setting) {
  return setting == AcSetting::StandbyTimeout ? kStandbyTimeout
                                               : kLidCloseAction;
}

bool parse_guid(const std::string& text, GUID& guid, std::string& error) {
  const std::wstring wide = widen(text);
  const HRESULT result = CLSIDFromString(wide.c_str(), &guid);
  if (SUCCEEDED(result)) return true;
  error = "invalid power scheme identifier";
  return false;
}

std::string guid_string(const GUID& guid) {
  std::array<wchar_t, 40> buffer{};
  if (StringFromGUID2(guid, buffer.data(), static_cast<int>(buffer.size())) <=
      0) {
    return {};
  }
  return narrow(buffer.data());
}

bool write_all(const HANDLE file, const std::string& content,
               std::string& error) {
  std::size_t offset = 0;
  while (offset < content.size()) {
    const auto remaining = std::min<std::size_t>(
        content.size() - offset, std::numeric_limits<DWORD>::max());
    DWORD written{};
    if (!WriteFile(file, content.data() + offset,
                   static_cast<DWORD>(remaining), &written, nullptr) ||
        written == 0) {
      error = windows_error(GetLastError());
      return false;
    }
    offset += written;
  }
  return true;
}

bool is_missing_file_error(const DWORD error) {
  return error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND;
}

void put_pixel(std::uint32_t* pixels, const int size, const int x, const int y,
               const std::uint32_t color) {
  if (x < 0 || y < 0 || x >= size || y >= size) return;
  pixels[y * size + x] = color;
}

void fill_rect(std::uint32_t* pixels, const int size, const int left,
               const int top, const int right, const int bottom,
               const std::uint32_t color) {
  for (int y = top; y <= bottom; ++y) {
    for (int x = left; x <= right; ++x) put_pixel(pixels, size, x, y, color);
  }
}

void draw_line(std::uint32_t* pixels, const int size, int x0, int y0,
               const int x1, const int y1, const std::uint32_t color,
               const int thickness = 2) {
  const int dx = std::abs(x1 - x0);
  const int sx = x0 < x1 ? 1 : -1;
  const int dy = -std::abs(y1 - y0);
  const int sy = y0 < y1 ? 1 : -1;
  int error = dx + dy;
  while (true) {
    fill_rect(pixels, size, x0 - thickness / 2, y0 - thickness / 2,
              x0 + thickness / 2, y0 + thickness / 2, color);
    if (x0 == x1 && y0 == y1) break;
    const int doubled = 2 * error;
    if (doubled >= dy) {
      error += dy;
      x0 += sx;
    }
    if (doubled <= dx) {
      error += dx;
      y0 += sy;
    }
  }
}

}  // namespace

std::wstring widen(const std::string& value) {
  if (value.empty()) return {};
  const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
                                       value.data(), static_cast<int>(value.size()),
                                       nullptr, 0);
  if (size <= 0) return {};
  std::wstring result(static_cast<std::size_t>(size), L'\0');
  MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
                      static_cast<int>(value.size()), result.data(), size);
  return result;
}

std::string narrow(const std::wstring& value) {
  if (value.empty()) return {};
  const int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
                                       value.data(), static_cast<int>(value.size()),
                                       nullptr, 0, nullptr, nullptr);
  if (size <= 0) return {};
  std::string result(static_cast<std::size_t>(size), '\0');
  WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
                      static_cast<int>(value.size()), result.data(), size,
                      nullptr, nullptr);
  return result;
}

std::string windows_error(const DWORD error_code) {
  wchar_t* buffer = nullptr;
  const DWORD length = FormatMessageW(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr, error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<wchar_t*>(&buffer), 0, nullptr);
  if (length == 0 || buffer == nullptr) {
    return "Windows error " + std::to_string(error_code);
  }
  std::wstring message(buffer, length);
  LocalFree(buffer);
  while (!message.empty() &&
         (message.back() == L'\r' || message.back() == L'\n' ||
          message.back() == L' ')) {
    message.pop_back();
  }
  return narrow(message) + " (" + std::to_string(error_code) + ")";
}

bool WindowsPowerPolicy::can_write(const AcSetting setting,
                                   std::string& error) {
  const DWORD result =
      PowerSettingAccessCheck(ACCESS_AC_POWER_SETTING_INDEX,
                              &setting_guid_for(setting));
  if (result == ERROR_SUCCESS) return true;
  error = windows_error(result);
  return false;
}

bool WindowsPowerPolicy::active_scheme(std::string& scheme_id,
                                       std::string& error) {
  GUID* scheme = nullptr;
  const DWORD result = PowerGetActiveScheme(nullptr, &scheme);
  if (result != ERROR_SUCCESS || scheme == nullptr) {
    error = windows_error(result);
    return false;
  }
  scheme_id = guid_string(*scheme);
  LocalFree(scheme);
  if (!scheme_id.empty()) return true;
  error = "Windows returned an invalid active power scheme identifier.";
  return false;
}

bool WindowsPowerPolicy::lid_present(bool& present, std::string& error) {
  SYSTEM_POWER_CAPABILITIES capabilities{};
  if (!GetPwrCapabilities(&capabilities)) {
    error = windows_error(GetLastError());
    return false;
  }
  present = capabilities.LidPresent != FALSE;
  return true;
}

bool WindowsPowerPolicy::read_ac(const std::string& scheme_id,
                                 const AcSetting setting,
                                 std::uint32_t& value, std::string& error) {
  GUID scheme{};
  if (!parse_guid(scheme_id, scheme, error)) return false;
  DWORD result_value{};
  const DWORD result = PowerReadACValueIndex(
      nullptr, &scheme, &subgroup_for(setting), &setting_guid_for(setting),
      &result_value);
  if (result != ERROR_SUCCESS) {
    error = windows_error(result);
    return false;
  }
  value = result_value;
  return true;
}

bool WindowsPowerPolicy::write_ac(const std::string& scheme_id,
                                  const AcSetting setting,
                                  const std::uint32_t value,
                                  std::string& error) {
  GUID scheme{};
  if (!parse_guid(scheme_id, scheme, error)) return false;
  const DWORD result = PowerWriteACValueIndex(
      nullptr, &scheme, &subgroup_for(setting), &setting_guid_for(setting),
      value);
  if (result == ERROR_SUCCESS) return true;
  error = windows_error(result);
  return false;
}

bool WindowsPowerPolicy::apply_if_active(const std::string& scheme_id,
                                         std::string& error) {
  std::string current;
  if (!active_scheme(current, error)) return false;
  if (current != scheme_id) return true;
  GUID scheme{};
  if (!parse_guid(scheme_id, scheme, error)) return false;
  const DWORD result = PowerSetActiveScheme(nullptr, &scheme);
  if (result == ERROR_SUCCESS) return true;
  error = windows_error(result);
  return false;
}

WindowsJournalStore::WindowsJournalStore() {
  PWSTR local_app_data = nullptr;
  const HRESULT result = SHGetKnownFolderPath(
      FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &local_app_data);
  if (FAILED(result) || local_app_data == nullptr) {
    startup_error_ = "The Local AppData directory is unavailable.";
    return;
  }
  directory_ = std::filesystem::path(local_app_data) / L"AgentNightWatch";
  CoTaskMemFree(local_app_data);
  journal_path_ = std::filesystem::path(directory_) / L"session-v1.journal";

  if (!CreateDirectoryW(directory_.c_str(), nullptr)) {
    const DWORD create_error = GetLastError();
    if (create_error != ERROR_ALREADY_EXISTS) {
      startup_error_ = windows_error(create_error);
      return;
    }
  }
  const DWORD attributes = GetFileAttributesW(directory_.c_str());
  if (attributes == INVALID_FILE_ATTRIBUTES ||
      (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0 ||
      (attributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0) {
    startup_error_ = "The Agent Night Watch data path is not a normal directory.";
  }
}

bool WindowsJournalStore::save(const SessionRecord& record,
                               std::string& error) {
  if (!ready()) {
    error = startup_error_;
    return false;
  }
  const std::wstring temporary =
      journal_path_ + L"." + widen(record.session_id) + L".tmp";
  const HANDLE file = CreateFileW(
      temporary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
      FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH | FILE_FLAG_OPEN_REPARSE_POINT,
      nullptr);
  if (file == INVALID_HANDLE_VALUE) {
    error = windows_error(GetLastError());
    return false;
  }
  const std::string content = serialize_session(record);
  const bool wrote = write_all(file, content, error);
  const bool flushed = wrote && FlushFileBuffers(file) != FALSE;
  if (wrote && !flushed) error = windows_error(GetLastError());
  CloseHandle(file);
  if (!wrote || !flushed) {
    DeleteFileW(temporary.c_str());
    return false;
  }
  if (!MoveFileExW(temporary.c_str(), journal_path_.c_str(),
                   MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
    error = windows_error(GetLastError());
    DeleteFileW(temporary.c_str());
    return false;
  }
  return true;
}

std::optional<SessionRecord> WindowsJournalStore::load(std::string& error) {
  error.clear();
  if (!ready()) {
    error = startup_error_;
    return std::nullopt;
  }
  const HANDLE file = CreateFileW(
      journal_path_.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
  if (file == INVALID_HANDLE_VALUE) {
    const DWORD open_error = GetLastError();
    if (!is_missing_file_error(open_error)) error = windows_error(open_error);
    return std::nullopt;
  }
  BY_HANDLE_FILE_INFORMATION information{};
  if (!GetFileInformationByHandle(file, &information) ||
      (information.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0 ||
      information.nFileSizeHigh != 0 || information.nFileSizeLow > 4096) {
    CloseHandle(file);
    error = "The recovery journal is not a safe regular file.";
    return std::nullopt;
  }
  std::string content(information.nFileSizeLow, '\0');
  DWORD read{};
  const bool read_ok = content.empty() ||
                       ReadFile(file, content.data(), information.nFileSizeLow,
                                &read, nullptr) != FALSE;
  CloseHandle(file);
  if (!read_ok || read != content.size()) {
    error = read_ok ? "The recovery journal was only partially read."
                    : windows_error(GetLastError());
    return std::nullopt;
  }
  const auto parsed = parse_session(content);
  if (!parsed) error = "The recovery journal failed validation.";
  return parsed;
}

bool WindowsJournalStore::clear(std::string& error) {
  error.clear();
  if (!ready()) {
    error = startup_error_;
    return false;
  }
  if (DeleteFileW(journal_path_.c_str())) return true;
  const DWORD delete_error = GetLastError();
  if (is_missing_file_error(delete_error)) return true;
  error = windows_error(delete_error);
  return false;
}

std::string make_guid_string() {
  GUID guid{};
  if (FAILED(CoCreateGuid(&guid))) return {};
  return guid_string(guid);
}

std::wstring executable_path() {
  std::vector<wchar_t> buffer(512);
  while (buffer.size() < 32768) {
    const DWORD length = GetModuleFileNameW(
        nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0) return {};
    if (length < buffer.size() - 1) return {buffer.data(), length};
    buffer.resize(buffer.size() * 2);
  }
  return {};
}

bool process_is_alive(const std::uint32_t process_id) {
  const HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, process_id);
  if (process == nullptr) return false;
  const DWORD result = WaitForSingleObject(process, 0);
  CloseHandle(process);
  return result == WAIT_TIMEOUT;
}

bool system_is_on_ac() {
  SYSTEM_POWER_CAPABILITIES capabilities{};
  if (GetPwrCapabilities(&capabilities) &&
      capabilities.SystemBatteriesPresent == FALSE) {
    return true;
  }
  SYSTEM_POWER_STATUS status{};
  if (!GetSystemPowerStatus(&status)) return false;
  return status.ACLineStatus == 1;
}

bool start_watchdog(const std::uint32_t owner_pid,
                    const std::string& instance_id, std::string& error) {
  const std::wstring executable = executable_path();
  if (executable.empty()) {
    error = "The executable path could not be resolved.";
    return false;
  }
  std::wstring command = L"\"" + executable + L"\" --watchdog " +
                         std::to_wstring(owner_pid) + L" \"" +
                         widen(instance_id) + L"\"";
  STARTUPINFOW startup{};
  startup.cb = sizeof(startup);
  PROCESS_INFORMATION process{};
  if (!CreateProcessW(nullptr, command.data(), nullptr, nullptr, FALSE,
                      CREATE_NO_WINDOW, nullptr, nullptr, &startup, &process)) {
    error = windows_error(GetLastError());
    return false;
  }
  CloseHandle(process.hThread);
  CloseHandle(process.hProcess);
  return true;
}

int run_watchdog(const std::uint32_t owner_pid,
                 const std::string& instance_id) {
  const HANDLE owner = OpenProcess(SYNCHRONIZE, FALSE, owner_pid);
  if (owner != nullptr) {
    WaitForSingleObject(owner, INFINITE);
    CloseHandle(owner);
  }
  WindowsPowerPolicy policy;
  WindowsJournalStore journal;
  if (!journal.ready()) return 2;
  const OperationResult restored =
      restore_session(policy, journal, instance_id, owner_pid);
  return restored.kind == OperationKind::Success ||
                 restored.kind == OperationKind::NoSession
             ? 0
             : 3;
}

bool acquire_system_required(HANDLE& request, std::string& error) {
  if (request != nullptr) return true;
  REASON_CONTEXT reason{};
  reason.Version = POWER_REQUEST_CONTEXT_VERSION;
  reason.Flags = POWER_REQUEST_CONTEXT_SIMPLE_STRING;
  reason.Reason.SimpleReasonString =
      const_cast<PWSTR>(L"Agent Night Watch is manually enabled");
  request = PowerCreateRequest(&reason);
  if (request == INVALID_HANDLE_VALUE || request == nullptr) {
    request = nullptr;
    error = windows_error(GetLastError());
    return false;
  }
  if (!PowerSetRequest(request, PowerRequestSystemRequired)) {
    error = windows_error(GetLastError());
    CloseHandle(request);
    request = nullptr;
    return false;
  }
  return true;
}

void release_system_required(HANDLE& request) {
  if (request == nullptr) return;
  PowerClearRequest(request, PowerRequestSystemRequired);
  CloseHandle(request);
  request = nullptr;
}

HICON create_coffee_icon(const TrayVisualState state) {
  constexpr int size = 32;
  BITMAPV5HEADER header{};
  header.bV5Size = sizeof(header);
  header.bV5Width = size;
  header.bV5Height = -size;
  header.bV5Planes = 1;
  header.bV5BitCount = 32;
  header.bV5Compression = BI_BITFIELDS;
  header.bV5RedMask = 0x00ff0000;
  header.bV5GreenMask = 0x0000ff00;
  header.bV5BlueMask = 0x000000ff;
  header.bV5AlphaMask = 0xff000000;

  void* raw_pixels = nullptr;
  const HDC screen = GetDC(nullptr);
  const HBITMAP color_bitmap = CreateDIBSection(
      screen, reinterpret_cast<BITMAPINFO*>(&header), DIB_RGB_COLORS,
      &raw_pixels, nullptr, 0);
  ReleaseDC(nullptr, screen);
  if (color_bitmap == nullptr || raw_pixels == nullptr) return nullptr;
  auto* pixels = static_cast<std::uint32_t*>(raw_pixels);
  std::fill(pixels, pixels + size * size, 0);

  std::uint32_t color = 0xff6b7280;
  if (state == TrayVisualState::Active) color = 0xffffa928;
  if (state == TrayVisualState::Paused) color = 0xff4da3ff;
  if (state == TrayVisualState::Warning) color = 0xffff5252;

  draw_line(pixels, size, 7, 15, 7, 25, color, 2);
  draw_line(pixels, size, 7, 25, 21, 25, color, 2);
  draw_line(pixels, size, 21, 25, 21, 15, color, 2);
  draw_line(pixels, size, 7, 15, 21, 15, color, 2);
  draw_line(pixels, size, 21, 17, 26, 17, color, 2);
  draw_line(pixels, size, 26, 17, 26, 22, color, 2);
  draw_line(pixels, size, 26, 22, 21, 23, color, 2);
  draw_line(pixels, size, 5, 28, 25, 28, color, 2);

  if (state == TrayVisualState::Active) {
    draw_line(pixels, size, 10, 12, 9, 9, color, 2);
    draw_line(pixels, size, 9, 9, 11, 6, color, 2);
    draw_line(pixels, size, 16, 12, 15, 9, color, 2);
    draw_line(pixels, size, 15, 9, 17, 5, color, 2);
  } else if (state == TrayVisualState::Paused) {
    fill_rect(pixels, size, 11, 7, 13, 12, color);
    fill_rect(pixels, size, 17, 7, 19, 12, color);
  } else if (state == TrayVisualState::Warning) {
    fill_rect(pixels, size, 14, 5, 16, 10, color);
    fill_rect(pixels, size, 14, 12, 16, 14, color);
  }

  const HBITMAP mask_bitmap = CreateBitmap(size, size, 1, 1, nullptr);
  ICONINFO icon_info{};
  icon_info.fIcon = TRUE;
  icon_info.hbmColor = color_bitmap;
  icon_info.hbmMask = mask_bitmap;
  const HICON icon = CreateIconIndirect(&icon_info);
  DeleteObject(mask_bitmap);
  DeleteObject(color_bitmap);
  return icon;
}

void lock_and_turn_off_displays(HWND owner) {
  if (!LockWorkStation()) return;
  SetTimer(owner, 2, 750, nullptr);
}

}  // namespace agent_night_watch::windows
