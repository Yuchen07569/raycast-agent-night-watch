#include "agent_night_watch/power_session.hpp"
#include "agent_night_watch/session.hpp"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <utility>

using agent_night_watch::AcSetting;
using agent_night_watch::JournalStore;
using agent_night_watch::OperationKind;
using agent_night_watch::PowerPolicy;
using agent_night_watch::SessionPhase;
using agent_night_watch::SessionRecord;

namespace {

int failures = 0;

#define CHECK(condition)                                                        \
  do {                                                                          \
    if (!(condition)) {                                                          \
      std::cerr << __FILE__ << ':' << __LINE__ << " check failed: "            \
                << #condition << '\n';                                           \
      ++failures;                                                                \
    }                                                                            \
  } while (false)

struct SchemeValues {
  std::uint32_t standby{900};
  std::uint32_t lid{1};
};

class FakePolicy final : public PowerPolicy {
 public:
  std::string active{"scheme-a"};
  bool has_lid{true};
  bool policy_blocked{false};
  bool fail_write_lid{false};
  bool fail_apply{false};
  int lid_reads{};
  int lid_writes{};
  int apply_calls{};
  std::map<std::string, SchemeValues> schemes{{"scheme-a", {}},
                                               {"scheme-b", {1200, 2}}};

  bool can_write(AcSetting, std::string& error) override {
    if (!policy_blocked) return true;
    error = "blocked by policy";
    return false;
  }

  bool active_scheme(std::string& scheme_id, std::string&) override {
    scheme_id = active;
    return true;
  }

  bool lid_present(bool& present, std::string&) override {
    present = has_lid;
    return true;
  }

  bool read_ac(const std::string& scheme_id, AcSetting setting,
               std::uint32_t& value, std::string& error) override {
    const auto found = schemes.find(scheme_id);
    if (found == schemes.end()) {
      error = "missing scheme";
      return false;
    }
    if (setting == AcSetting::LidCloseAction) {
      ++lid_reads;
      value = found->second.lid;
    } else {
      value = found->second.standby;
    }
    return true;
  }

  bool write_ac(const std::string& scheme_id, AcSetting setting,
                std::uint32_t value, std::string& error) override {
    auto found = schemes.find(scheme_id);
    if (found == schemes.end()) {
      error = "missing scheme";
      return false;
    }
    if (setting == AcSetting::LidCloseAction) {
      ++lid_writes;
      if (fail_write_lid) {
        error = "simulated lid failure";
        return false;
      }
      found->second.lid = value;
    } else {
      found->second.standby = value;
    }
    return true;
  }

  bool apply_if_active(const std::string&, std::string& error) override {
    ++apply_calls;
    if (!fail_apply) return true;
    error = "simulated apply failure";
    return false;
  }
};

class FakeJournal final : public JournalStore {
 public:
  std::optional<SessionRecord> record;
  std::string corrupt_error;
  bool fail_save{false};

  bool save(const SessionRecord& value, std::string& error) override {
    if (fail_save) {
      error = "simulated journal failure";
      return false;
    }
    record = value;
    return true;
  }

  std::optional<SessionRecord> load(std::string& error) override {
    error = corrupt_error;
    return record;
  }

  bool clear(std::string&) override {
    record.reset();
    corrupt_error.clear();
    return true;
  }
};

SessionRecord sample_record() {
  return {
      .session_id = "session-1",
      .instance_id = "instance-1",
      .owner_pid = 4242,
      .scheme_id = "scheme-a",
      .standby_ac = 900,
      .lid_present = true,
      .lid_ac = 1,
      .phase = SessionPhase::Active,
  };
}

void session_round_trip() {
  const SessionRecord record = sample_record();
  const std::string serialized = agent_night_watch::serialize_session(record);
  const auto parsed = agent_night_watch::parse_session(serialized);
  CHECK(parsed.has_value());
  CHECK(*parsed == record);
}

void session_rejects_corruption() {
  std::string serialized = agent_night_watch::serialize_session(sample_record());
  serialized[serialized.find("standby_ac=900") + 11] = '1';
  CHECK(!agent_night_watch::parse_session(serialized));
  CHECK(!agent_night_watch::parse_session("format=unknown\n"));
}

void enable_and_restore_laptop() {
  FakePolicy policy;
  FakeJournal journal;
  const auto enabled = agent_night_watch::enable_session(
      policy, journal, "session-1", "instance-1", 4242);
  CHECK(enabled.ok());
  CHECK(policy.schemes["scheme-a"].standby == 0);
  CHECK(policy.schemes["scheme-a"].lid == 0);
  CHECK(journal.record && journal.record->phase == SessionPhase::Active);

  const auto restored = agent_night_watch::restore_session(policy, journal);
  CHECK(restored.ok());
  CHECK(policy.schemes["scheme-a"].standby == 900);
  CHECK(policy.schemes["scheme-a"].lid == 1);
  CHECK(!journal.record);
}

void desktop_never_touches_lid() {
  FakePolicy policy;
  FakeJournal journal;
  policy.has_lid = false;
  const auto enabled = agent_night_watch::enable_session(
      policy, journal, "session-1", "instance-1", 4242);
  CHECK(enabled.ok());
  CHECK(policy.lid_reads == 0);
  CHECK(policy.lid_writes == 0);
  const auto restored = agent_night_watch::restore_session(policy, journal);
  CHECK(restored.ok());
  CHECK(policy.lid_reads == 0);
  CHECK(policy.lid_writes == 0);
}

void blocked_policy_changes_nothing() {
  FakePolicy policy;
  FakeJournal journal;
  policy.policy_blocked = true;
  const auto result = agent_night_watch::enable_session(
      policy, journal, "session-1", "instance-1", 4242);
  CHECK(result.kind == OperationKind::Failure);
  CHECK(policy.schemes["scheme-a"].standby == 900);
  CHECK(policy.schemes["scheme-a"].lid == 1);
  CHECK(!journal.record);
}

void partial_enable_rolls_back() {
  FakePolicy policy;
  FakeJournal journal;
  policy.fail_write_lid = true;
  const auto result = agent_night_watch::enable_session(
      policy, journal, "session-1", "instance-1", 4242);
  CHECK(result.kind == OperationKind::Failure);
  CHECK(policy.schemes["scheme-a"].standby == 900);
  CHECK(policy.schemes["scheme-a"].lid == 1);
  CHECK(!journal.record);
}

void active_scheme_change_is_warning() {
  FakePolicy policy;
  FakeJournal journal;
  CHECK(agent_night_watch::enable_session(policy, journal, "session-1",
                                          "instance-1", 4242)
            .ok());
  policy.active = "scheme-b";
  const auto status = agent_night_watch::validate_owned_session(policy, journal);
  CHECK(status.kind == OperationKind::ExternalChange);
}

void external_change_is_not_overwritten() {
  FakePolicy policy;
  FakeJournal journal;
  CHECK(agent_night_watch::enable_session(policy, journal, "session-1",
                                          "instance-1", 4242)
            .ok());
  policy.schemes["scheme-a"].standby = 300;
  const auto restored = agent_night_watch::restore_session(policy, journal);
  CHECK(restored.kind == OperationKind::ExternalChange);
  CHECK(policy.schemes["scheme-a"].standby == 300);
  CHECK(policy.schemes["scheme-a"].lid == 0);
  CHECK(journal.record.has_value());
}

void old_watchdog_cannot_restore_new_instance() {
  FakePolicy policy;
  FakeJournal journal;
  journal.record = sample_record();
  policy.schemes["scheme-a"] = {0, 0};
  const auto restored = agent_night_watch::restore_session(
      policy, journal, std::string("old-instance"), 4242);
  CHECK(restored.kind == OperationKind::ExternalChange);
  CHECK(policy.schemes["scheme-a"].standby == 0);
  CHECK(policy.schemes["scheme-a"].lid == 0);
}

void corrupt_journal_blocks_enable() {
  FakePolicy policy;
  FakeJournal journal;
  journal.corrupt_error = "checksum mismatch";
  const auto result = agent_night_watch::enable_session(
      policy, journal, "session-1", "instance-1", 4242);
  CHECK(result.kind == OperationKind::Failure);
  CHECK(policy.schemes["scheme-a"].standby == 900);
}

void already_customized_values_survive_round_trip() {
  FakePolicy policy;
  FakeJournal journal;
  policy.schemes["scheme-a"] = {0, 0};
  CHECK(agent_night_watch::enable_session(policy, journal, "session-1",
                                          "instance-1", 4242)
            .ok());
  CHECK(agent_night_watch::restore_session(policy, journal).ok());
  CHECK(policy.schemes["scheme-a"].standby == 0);
  CHECK(policy.schemes["scheme-a"].lid == 0);
}

}  // namespace

int main() {
  session_round_trip();
  session_rejects_corruption();
  enable_and_restore_laptop();
  desktop_never_touches_lid();
  blocked_policy_changes_nothing();
  partial_enable_rolls_back();
  active_scheme_change_is_warning();
  external_change_is_not_overwritten();
  old_watchdog_cannot_restore_new_instance();
  corrupt_journal_blocks_enable();
  already_customized_values_survive_round_trip();

  if (failures != 0) {
    std::cerr << failures << " test checks failed\n";
    return EXIT_FAILURE;
  }
  std::cout << "All Agent Night Watch core tests passed\n";
  return EXIT_SUCCESS;
}
