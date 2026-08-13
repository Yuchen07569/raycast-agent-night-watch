#!/bin/sh

set -eu

json_escape() {
  /usr/bin/sed 's/\\/\\\\/g; s/"/\\"/g' <<EOF
$1
EOF
}

platform=$(/usr/bin/uname -s 2>/dev/null || /bin/echo unknown)
architecture=$(/usr/bin/uname -m 2>/dev/null || /bin/echo unknown)

if [ "$platform" != "Darwin" ]; then
  /usr/bin/printf '{"supported":false,"platform":"%s","reason":"Agent Night Watch requires macOS"}\n' "$(json_escape "$platform")"
  exit 2
fi

raycast_installed=false
raycast_variant="none"
raycast_version="unknown"

for candidate in \
  "/Applications/Raycast.app" \
  "/Applications/Raycast Beta.app" \
  "$HOME/Applications/Raycast.app" \
  "$HOME/Applications/Raycast Beta.app"
do
  if [ -d "$candidate" ]; then
    raycast_installed=true
    case "$candidate" in
      *"Raycast Beta.app") raycast_variant="Beta" ;;
      *) raycast_variant="Stable" ;;
    esac
    version=$(/usr/libexec/PlistBuddy -c 'Print :CFBundleShortVersionString' "$candidate/Contents/Info.plist" 2>/dev/null || true)
    if [ -n "$version" ]; then raycast_version=$version; fi
    break
  fi
done

sleep_disabled=$(/usr/bin/pmset -g 2>/dev/null | /usr/bin/awk '$1 == "SleepDisabled" { print $2; exit }')
case "$sleep_disabled" in
  0|1) ;;
  *) sleep_disabled="null" ;;
esac

session_file="$HOME/Library/Caches/com.yuchen.agent-night-watch/session.json"
session_cache="missing"
session_phase="none"
if [ -f "$session_file" ]; then
  session_cache="present"
  phase=$(/usr/bin/sed -n 's/.*"phase"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p' "$session_file" | /usr/bin/head -n 1)
  if [ -n "$phase" ]; then session_phase=$phase; fi
fi

/usr/bin/printf '%s\n' "{\"supported\":true,\"platform\":\"macOS\",\"architecture\":\"$(json_escape "$architecture")\",\"raycast\":{\"installed\":$raycast_installed,\"variant\":\"$(json_escape "$raycast_variant")\",\"version\":\"$(json_escape "$raycast_version")\"},\"sleepDisabled\":$sleep_disabled,\"extensionSessionCache\":\"$(json_escape "$session_cache")\",\"extensionSessionPhase\":\"$(json_escape "$session_phase")\"}"
