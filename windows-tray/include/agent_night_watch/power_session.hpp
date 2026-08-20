#pragma once

#include "agent_night_watch/session.hpp"

#include <cstdint>
#include <optional>
#include <string>

namespace agent_night_watch {

enum class AcSetting {
  StandbyTimeout,
  LidCloseAction,
};

class PowerPolicy {
 public:
  virtual ~PowerPolicy() = default;

  virtual bool can_write(AcSetting setting, std::string& error) = 0;
  virtual bool active_scheme(std::string& scheme_id, std::string& error) = 0;
  virtual bool lid_present(bool& present, std::string& error) = 0;
  virtual bool read_ac(const std::string& scheme_id, AcSetting setting,
                       std::uint32_t& value, std::string& error) = 0;
  virtual bool write_ac(const std::string& scheme_id, AcSetting setting,
                        std::uint32_t value, std::string& error) = 0;
  virtual bool apply_if_active(const std::string& scheme_id,
                               std::string& error) = 0;
};

class JournalStore {
 public:
  virtual ~JournalStore() = default;

  virtual bool save(const SessionRecord& record, std::string& error) = 0;
  virtual std::optional<SessionRecord> load(std::string& error) = 0;
  virtual bool clear(std::string& error) = 0;
};

enum class OperationKind {
  Success,
  Failure,
  ExternalChange,
  NoSession,
};

struct OperationResult {
  OperationKind kind{OperationKind::Failure};
  std::string message;
  std::optional<SessionRecord> session;

  [[nodiscard]] bool ok() const { return kind == OperationKind::Success; }
};

OperationResult enable_session(PowerPolicy& policy, JournalStore& journal,
                               const std::string& session_id,
                               const std::string& instance_id,
                               std::uint32_t owner_pid);

OperationResult restore_session(
    PowerPolicy& policy, JournalStore& journal,
    const std::optional<std::string>& expected_instance_id = std::nullopt,
    const std::optional<std::uint32_t>& expected_owner_pid = std::nullopt);

OperationResult validate_owned_session(PowerPolicy& policy,
                                       JournalStore& journal);

}  // namespace agent_night_watch
