---
name: agent-night-watch
description: Install, inspect, update, uninstall safely, or troubleshoot Agent Night Watch on macOS or Windows. Use only when the user explicitly invokes $agent-night-watch or specifically asks to maintain Agent Night Watch; do not use it to change sleep settings or operate the coffee-cup control for the user.
---

# Agent Night Watch

Maintain the macOS Raycast extension or Windows portable tray utility without
taking control of the user's sleep state.

## Non-negotiable boundary

- Never run `pmset`, `powercfg`, PowerShell, Win32 power APIs, `sudo`, or a
  privileged AppleScript in a mode that writes power settings.
- Never click the coffee-cup control or press its hotkey for the user.
- Never install a privileged helper, launch agent, daemon, or sudoers rule.
- Use only the provided read-only platform preflight for status verification.
- Ask the user to operate the coffee cup or their Raycast hotkey, then verify
  the resulting state if requested.

## Start every task

Detect the current platform, then run the matching preflight from this Skill
directory:

- macOS: `/bin/sh scripts/preflight.sh`
- Windows: `powershell -NoProfile -ExecutionPolicy Bypass -File scripts/preflight.ps1`

Stop on any other platform. Summarize the system power values, Agent Night
Watch process/session state, and platform-specific host availability. A cache
or journal never replaces system power values as truth.

## Route the request

- **Install, update, or uninstall:** Read `references/operations.md` and follow
  the matching workflow.
- **Incorrect icon, failed toggle, stale state, or external ownership:** Read
  `references/troubleshooting.md`.
- **Permission, safety, or provenance question:** Read
  `references/security-and-provenance.md`.

Prefer the Raycast Store for macOS and Microsoft Store for Windows once their
official listings exist. Before that, use only named releases from the
maintainer repository and verify published checksums. Never use executables
from forks, comments, mirrors, or unrelated repositories.

## Report accurately

On macOS, separate these facts:

1. `SleepDisabled=1` means system sleep is disabled.
2. Valid extension session state means Agent Night Watch appears to own it.
3. Neither fact guarantees network access, process health, battery capacity, or
   safe temperature.

On Windows, separate these facts:

1. AC standby timeout `0` means Never for the recorded power scheme.
2. AC lid action `0` means Do Nothing, but only matters on a device with a lid.
3. A valid journal plus a running tray process indicates apparent ownership.
4. Battery values are intentionally outside Agent Night Watch ownership.

Only report checks actually performed. Treat a real closed-lid test as a manual
release gate, not something inferred from an open-lid check.
