#include "agent_night_watch/power_session.hpp"

#include <utility>

namespace agent_night_watch {
namespace {

OperationResult failure(std::string message,
                        std::optional<SessionRecord> session = std::nullopt) {
  return {OperationKind::Failure, std::move(message), std::move(session)};
}

bool rollback_apply(PowerPolicy& policy, const SessionRecord& record,
                    const bool standby_written, const bool lid_written,
                    std::string& error) {
  std::string rollback_error;
  bool ok = true;
  if (standby_written &&
      !policy.write_ac(record.scheme_id, AcSetting::StandbyTimeout,
                       record.standby_ac, rollback_error)) {
    ok = false;
    error += " Rollback of the sleep timeout failed: " + rollback_error;
  }
  if (lid_written &&
      !policy.write_ac(record.scheme_id, AcSetting::LidCloseAction,
                       record.lid_ac, rollback_error)) {
    ok = false;
    error += " Rollback of the lid action failed: " + rollback_error;
  }
  if (!policy.apply_if_active(record.scheme_id, rollback_error)) {
    ok = false;
    error += " Reapplying the original power scheme failed: " + rollback_error;
  }
  return ok;
}

}  // namespace

OperationResult enable_session(PowerPolicy& policy, JournalStore& journal,
                               const std::string& session_id,
                               const std::string& instance_id,
                               const std::uint32_t owner_pid) {
  std::string error;
  std::string existing_error;
  if (const auto existing = journal.load(existing_error); existing) {
    return failure("An unfinished Agent Night Watch session already exists.",
                   existing);
  }
  if (!existing_error.empty()) {
    return failure("The existing recovery journal is unreadable: " +
                   existing_error);
  }
  if (!policy.can_write(AcSetting::StandbyTimeout, error)) {
    return failure("The AC sleep timeout cannot be changed: " + error);
  }

  SessionRecord record{
      .session_id = session_id,
      .instance_id = instance_id,
      .owner_pid = owner_pid,
      .phase = SessionPhase::Prepared,
  };
  if (!policy.active_scheme(record.scheme_id, error)) {
    return failure("The active power scheme could not be read: " + error);
  }
  if (!policy.lid_present(record.lid_present, error)) {
    return failure("The system's lid capability could not be read: " + error);
  }
  if (record.lid_present &&
      !policy.can_write(AcSetting::LidCloseAction, error)) {
    return failure("The AC lid action cannot be changed: " + error);
  }
  if (!policy.read_ac(record.scheme_id, AcSetting::StandbyTimeout,
                      record.standby_ac, error)) {
    return failure("The original AC sleep timeout could not be read: " + error);
  }
  if (record.lid_present &&
      !policy.read_ac(record.scheme_id, AcSetting::LidCloseAction,
                      record.lid_ac, error)) {
    return failure("The original AC lid action could not be read: " + error);
  }
  if (!journal.save(record, error)) {
    return failure("The recovery journal could not be created: " + error);
  }

  bool standby_written = false;
  bool lid_written = false;
  if (record.standby_ac != 0) {
    if (!policy.write_ac(record.scheme_id, AcSetting::StandbyTimeout, 0,
                         error)) {
      std::string clear_error;
      journal.clear(clear_error);
      return failure("The AC sleep timeout could not be changed: " + error,
                     record);
    }
    standby_written = true;
  }
  if (record.lid_present && record.lid_ac != 0) {
    if (!policy.write_ac(record.scheme_id, AcSetting::LidCloseAction, 0,
                         error)) {
      rollback_apply(policy, record, standby_written, false, error);
      std::string clear_error;
      journal.clear(clear_error);
      return failure("The AC lid action could not be changed: " + error,
                     record);
    }
    lid_written = true;
  }
  if (!policy.apply_if_active(record.scheme_id, error)) {
    rollback_apply(policy, record, standby_written, lid_written, error);
    std::string clear_error;
    journal.clear(clear_error);
    return failure("The changed power scheme could not be activated: " + error,
                   record);
  }

  std::uint32_t verified{};
  if (!policy.read_ac(record.scheme_id, AcSetting::StandbyTimeout, verified,
                      error) ||
      verified != 0) {
    rollback_apply(policy, record, standby_written, lid_written, error);
    std::string clear_error;
    journal.clear(clear_error);
    return failure("The AC sleep timeout did not verify as Never: " + error,
                   record);
  }
  if (record.lid_present &&
      (!policy.read_ac(record.scheme_id, AcSetting::LidCloseAction, verified,
                       error) ||
       verified != 0)) {
    rollback_apply(policy, record, standby_written, lid_written, error);
    std::string clear_error;
    journal.clear(clear_error);
    return failure("The AC lid action did not verify as Do Nothing: " + error,
                   record);
  }

  record.phase = SessionPhase::Active;
  if (!journal.save(record, error)) {
    rollback_apply(policy, record, standby_written, lid_written, error);
    std::string clear_error;
    journal.clear(clear_error);
    return failure("The active recovery journal could not be committed: " +
                       error,
                   record);
  }
  return {OperationKind::Success, "AC Night Watch settings are active.", record};
}

OperationResult restore_session(
    PowerPolicy& policy, JournalStore& journal,
    const std::optional<std::string>& expected_instance_id,
    const std::optional<std::uint32_t>& expected_owner_pid) {
  std::string error;
  const auto loaded = journal.load(error);
  if (!loaded) {
    if (!error.empty()) return failure("The recovery journal is unreadable: " + error);
    return {OperationKind::NoSession, "No owned session needs restoration.",
            std::nullopt};
  }
  SessionRecord record = *loaded;
  if ((expected_instance_id && record.instance_id != *expected_instance_id) ||
      (expected_owner_pid && record.owner_pid != *expected_owner_pid)) {
    return {OperationKind::ExternalChange,
            "The recovery journal belongs to a different application instance.",
            record};
  }

  std::uint32_t current_standby{};
  std::uint32_t current_lid{};
  if (!policy.read_ac(record.scheme_id, AcSetting::StandbyTimeout,
                      current_standby, error)) {
    return failure("The current AC sleep timeout could not be read: " + error,
                   record);
  }
  if (record.lid_present &&
      !policy.read_ac(record.scheme_id, AcSetting::LidCloseAction, current_lid,
                      error)) {
    return failure("The current AC lid action could not be read: " + error,
                   record);
  }

  const bool standby_owned = current_standby == 0 ||
                             current_standby == record.standby_ac;
  const bool lid_owned = !record.lid_present || current_lid == 0 ||
                         current_lid == record.lid_ac;
  if (!standby_owned || !lid_owned) {
    return {OperationKind::ExternalChange,
            "Power settings changed outside Agent Night Watch; no values were overwritten.",
            record};
  }

  record.phase = SessionPhase::Restoring;
  if (!journal.save(record, error)) {
    return failure("The recovery journal could not enter restore mode: " + error,
                   record);
  }
  if (current_standby != record.standby_ac &&
      !policy.write_ac(record.scheme_id, AcSetting::StandbyTimeout,
                       record.standby_ac, error)) {
    return failure("The original AC sleep timeout could not be restored: " + error,
                   record);
  }
  if (record.lid_present && current_lid != record.lid_ac &&
      !policy.write_ac(record.scheme_id, AcSetting::LidCloseAction,
                       record.lid_ac, error)) {
    return failure("The original AC lid action could not be restored: " + error,
                   record);
  }
  if (!policy.apply_if_active(record.scheme_id, error)) {
    return failure("The restored power scheme could not be applied: " + error,
                   record);
  }

  std::uint32_t verified{};
  if (!policy.read_ac(record.scheme_id, AcSetting::StandbyTimeout, verified,
                      error) ||
      verified != record.standby_ac) {
    return failure("The original AC sleep timeout did not verify after restore: " +
                       error,
                   record);
  }
  if (record.lid_present &&
      (!policy.read_ac(record.scheme_id, AcSetting::LidCloseAction, verified,
                       error) ||
       verified != record.lid_ac)) {
    return failure("The original AC lid action did not verify after restore: " +
                       error,
                   record);
  }
  if (!journal.clear(error)) {
    return failure("Settings were restored, but the recovery journal remains: " +
                       error,
                   record);
  }
  return {OperationKind::Success, "Original AC power settings were restored.",
          record};
}

OperationResult validate_owned_session(PowerPolicy& policy,
                                       JournalStore& journal) {
  std::string error;
  const auto loaded = journal.load(error);
  if (!loaded) {
    if (!error.empty()) return failure("The recovery journal is unreadable: " + error);
    return {OperationKind::NoSession, "Night Watch is off.", std::nullopt};
  }
  const SessionRecord& record = *loaded;
  std::string active_scheme;
  if (!policy.active_scheme(active_scheme, error)) {
    return failure("The active power scheme could not be read: " + error, record);
  }
  if (active_scheme != record.scheme_id) {
    return {OperationKind::ExternalChange,
            "The active power scheme changed while Night Watch was enabled.",
            record};
  }
  std::uint32_t value{};
  if (!policy.read_ac(record.scheme_id, AcSetting::StandbyTimeout, value,
                      error)) {
    return failure("The AC sleep timeout could not be verified: " + error, record);
  }
  if (value != 0) {
    return {OperationKind::ExternalChange,
            "The AC sleep timeout changed while Night Watch was enabled.",
            record};
  }
  if (record.lid_present) {
    if (!policy.read_ac(record.scheme_id, AcSetting::LidCloseAction, value,
                        error)) {
      return failure("The AC lid action could not be verified: " + error, record);
    }
    if (value != 0) {
      return {OperationKind::ExternalChange,
              "The AC lid action changed while Night Watch was enabled.",
              record};
    }
  }
  return {OperationKind::Success, "Owned AC Night Watch settings are active.",
          record};
}

}  // namespace agent_night_watch
