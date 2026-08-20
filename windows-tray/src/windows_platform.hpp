#pragma once

#include "agent_night_watch/power_session.hpp"

#include <windows.h>

#include <cstdint>
#include <optional>
#include <string>

namespace agent_night_watch::windows {

enum class TrayVisualState {
  Off,
  Active,
  Paused,
  Warning,
};

class WindowsPowerPolicy final : public PowerPolicy {
 public:
  bool can_write(AcSetting setting, std::string& error) override;
  bool active_scheme(std::string& scheme_id, std::string& error) override;
  bool lid_present(bool& present, std::string& error) override;
  bool read_ac(const std::string& scheme_id, AcSetting setting,
               std::uint32_t& value, std::string& error) override;
  bool write_ac(const std::string& scheme_id, AcSetting setting,
                std::uint32_t value, std::string& error) override;
  bool apply_if_active(const std::string& scheme_id,
                       std::string& error) override;
};

class WindowsJournalStore final : public JournalStore {
 public:
  WindowsJournalStore();

  [[nodiscard]] bool ready() const { return startup_error_.empty(); }
  [[nodiscard]] const std::string& startup_error() const {
    return startup_error_;
  }
  [[nodiscard]] const std::wstring& path() const { return journal_path_; }

  bool save(const SessionRecord& record, std::string& error) override;
  std::optional<SessionRecord> load(std::string& error) override;
  bool clear(std::string& error) override;

 private:
  std::wstring directory_;
  std::wstring journal_path_;
  std::string startup_error_;
};

std::string make_guid_string();
std::wstring executable_path();
bool process_is_alive(std::uint32_t process_id);
bool system_is_on_ac();

bool start_watchdog(std::uint32_t owner_pid, const std::string& instance_id,
                    std::string& error);
int run_watchdog(std::uint32_t owner_pid, const std::string& instance_id);

bool acquire_system_required(HANDLE& request, std::string& error);
void release_system_required(HANDLE& request);

HICON create_coffee_icon(TrayVisualState state);
void lock_and_turn_off_displays(HWND owner);

std::wstring widen(const std::string& value);
std::string narrow(const std::wstring& value);
std::string windows_error(DWORD error_code);

}  // namespace agent_night_watch::windows
