#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace agent_night_watch {

enum class SessionPhase {
  Prepared,
  Active,
  Restoring,
};

struct SessionRecord {
  std::string session_id;
  std::string instance_id;
  std::uint32_t owner_pid{};
  std::string scheme_id;
  std::uint32_t standby_ac{};
  bool lid_present{};
  std::uint32_t lid_ac{};
  SessionPhase phase{SessionPhase::Prepared};

  bool operator==(const SessionRecord&) const = default;
};

std::string serialize_session(const SessionRecord& record);
std::optional<SessionRecord> parse_session(const std::string& serialized);
std::string phase_name(SessionPhase phase);

}  // namespace agent_night_watch
