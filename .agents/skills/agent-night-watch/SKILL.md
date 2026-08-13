---
name: agent-night-watch
description: Install, inspect, update, uninstall safely, or troubleshoot the Agent Night Watch Raycast extension for manual MacBook closed-lid awake control. Use only when the user explicitly invokes $agent-night-watch or specifically asks to install, verify, update, remove, or diagnose Agent Night Watch; do not use it to turn macOS sleep on or off for the user.
---

# Agent Night Watch

Maintain the Raycast extension without taking control of the user's sleep state.

## Non-negotiable boundary

- Never run `pmset -a disablesleep`, `sudo`, a privileged AppleScript, or any
  equivalent write operation.
- Never click the coffee-cup control or press its hotkey for the user.
- Never install a privileged helper, launch agent, daemon, or sudoers rule.
- Use `pmset -g` only for read-only verification.
- Ask the user to operate the coffee cup or their Raycast hotkey, then verify
  the resulting state if requested.

## Start every task

Run `/bin/sh scripts/preflight.sh` from this Skill directory. If it reports a
non-macOS platform, stop. Summarize
Raycast availability, `SleepDisabled`, and whether Agent Night Watch has local
session state. Do not reinterpret cache state as system truth.

## Route the request

- **Install, update, or uninstall:** Read `references/operations.md` and follow
  the matching workflow.
- **Incorrect icon, failed toggle, stale state, or external ownership:** Read
  `references/troubleshooting.md`.
- **Permission, safety, or provenance question:** Read
  `references/security-and-provenance.md`.

Prefer the Raycast Store once its official listing exists. Until then, use the
tagged GitHub source instructions. Do not download executables from releases or
unrelated repositories.

## Report accurately

Separate these facts:

1. `SleepDisabled=1` means system sleep is disabled.
2. Valid extension session state means Agent Night Watch appears to own it.
3. Neither fact guarantees network access, process health, battery capacity, or
   safe temperature.

Only report checks actually performed. Treat a real closed-lid test as a manual
release gate, not something inferred from an open-lid check.
