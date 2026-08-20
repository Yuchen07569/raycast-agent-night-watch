#include "agent_night_watch/session.hpp"

#include <charconv>
#include <iomanip>
#include <map>
#include <sstream>
#include <string_view>

namespace agent_night_watch {
namespace {

constexpr std::string_view kFormat = "agent-night-watch-session-v1";

std::uint64_t fnv1a(std::string_view text) {
  std::uint64_t value = 14695981039346656037ULL;
  for (const unsigned char byte : text) {
    value ^= byte;
    value *= 1099511628211ULL;
  }
  return value;
}

bool safe_text(const std::string& value, std::size_t maximum) {
  if (value.empty() || value.size() > maximum) return false;
  for (const unsigned char character : value) {
    if (character < 0x20 || character == '=' || character == 0x7f) return false;
  }
  return true;
}

template <typename Integer>
std::optional<Integer> parse_integer(std::string_view text) {
  Integer value{};
  const auto result = std::from_chars(text.data(), text.data() + text.size(), value);
  if (result.ec != std::errc{} || result.ptr != text.data() + text.size()) {
    return std::nullopt;
  }
  return value;
}

std::optional<SessionPhase> parse_phase(std::string_view value) {
  if (value == "prepared") return SessionPhase::Prepared;
  if (value == "active") return SessionPhase::Active;
  if (value == "restoring") return SessionPhase::Restoring;
  return std::nullopt;
}

}  // namespace

std::string phase_name(const SessionPhase phase) {
  switch (phase) {
    case SessionPhase::Prepared:
      return "prepared";
    case SessionPhase::Active:
      return "active";
    case SessionPhase::Restoring:
      return "restoring";
  }
  return "invalid";
}

std::string serialize_session(const SessionRecord& record) {
  std::ostringstream body;
  body << "format=" << kFormat << '\n'
       << "session_id=" << record.session_id << '\n'
       << "instance_id=" << record.instance_id << '\n'
       << "owner_pid=" << record.owner_pid << '\n'
       << "scheme_id=" << record.scheme_id << '\n'
       << "standby_ac=" << record.standby_ac << '\n'
       << "lid_present=" << (record.lid_present ? 1 : 0) << '\n'
       << "lid_ac=" << record.lid_ac << '\n'
       << "phase=" << phase_name(record.phase) << '\n';

  std::ostringstream output;
  const std::string body_text = body.str();
  output << body_text << "checksum=" << std::hex << std::setw(16)
         << std::setfill('0') << fnv1a(body_text) << '\n';
  return output.str();
}

std::optional<SessionRecord> parse_session(const std::string& serialized) {
  if (serialized.size() > 4096 || serialized.empty()) return std::nullopt;

  const auto checksum_position = serialized.rfind("checksum=");
  if (checksum_position == std::string::npos || checksum_position == 0) {
    return std::nullopt;
  }
  const std::string_view body(serialized.data(), checksum_position);
  const std::size_t checksum_end = serialized.find('\n', checksum_position);
  if (checksum_end == std::string::npos || checksum_end + 1 != serialized.size()) {
    return std::nullopt;
  }
  const std::string_view checksum_text(
      serialized.data() + checksum_position + 9,
      checksum_end - checksum_position - 9);
  if (checksum_text.size() != 16) return std::nullopt;

  std::uint64_t checksum{};
  const auto checksum_result = std::from_chars(
      checksum_text.data(), checksum_text.data() + checksum_text.size(), checksum,
      16);
  if (checksum_result.ec != std::errc{} ||
      checksum_result.ptr != checksum_text.data() + checksum_text.size() ||
      checksum != fnv1a(body)) {
    return std::nullopt;
  }

  std::map<std::string, std::string> fields;
  std::size_t start = 0;
  while (start < body.size()) {
    const std::size_t end = body.find('\n', start);
    if (end == std::string_view::npos) return std::nullopt;
    const std::string_view line = body.substr(start, end - start);
    const std::size_t separator = line.find('=');
    if (separator == std::string_view::npos || separator == 0) {
      return std::nullopt;
    }
    const std::string key(line.substr(0, separator));
    const std::string value(line.substr(separator + 1));
    if (!fields.emplace(key, value).second) return std::nullopt;
    start = end + 1;
  }

  if (fields.size() != 9 || fields["format"] != kFormat) return std::nullopt;
  const auto owner_pid = parse_integer<std::uint32_t>(fields["owner_pid"]);
  const auto standby_ac = parse_integer<std::uint32_t>(fields["standby_ac"]);
  const auto lid_present = parse_integer<unsigned int>(fields["lid_present"]);
  const auto lid_ac = parse_integer<std::uint32_t>(fields["lid_ac"]);
  const auto phase = parse_phase(fields["phase"]);
  if (!owner_pid || *owner_pid <= 1 || !standby_ac || !lid_present ||
      *lid_present > 1 || !lid_ac || !phase ||
      !safe_text(fields["session_id"], 128) ||
      !safe_text(fields["instance_id"], 128) ||
      !safe_text(fields["scheme_id"], 128)) {
    return std::nullopt;
  }

  return SessionRecord{
      .session_id = fields["session_id"],
      .instance_id = fields["instance_id"],
      .owner_pid = *owner_pid,
      .scheme_id = fields["scheme_id"],
      .standby_ac = *standby_ac,
      .lid_present = *lid_present == 1,
      .lid_ac = *lid_ac,
      .phase = *phase,
  };
}

}  // namespace agent_night_watch
