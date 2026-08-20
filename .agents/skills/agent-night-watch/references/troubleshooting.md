# Troubleshooting

Use the platform reported by preflight. Never translate a Windows journal into
macOS ownership or a macOS cache into Windows ownership.

## macOS

## Coffee cup shows the wrong state

1. Run `/bin/sh scripts/preflight.sh` from the Skill directory and treat
   `SleepDisabled` as authoritative.
2. Ask the user to run **Night Watch Menu Bar** from Raycast root search once
   to refresh the rendered cup without changing sleep state.
3. In normal states, clicking the cup toggles directly and has no second menu.
   A recovery/status menu appears only for external ownership or transitions.
4. If `SleepDisabled=1` without valid owned state, report external ownership.
   Do not claim Agent Night Watch is active.

## Enable was canceled

Confirm that `SleepDisabled=0` and no valid session remains. Cancellation is a
normal outcome; do not retry automatically or ask for a password outside the
macOS dialog.

## Disable did not finish

If the extension reports an owned session, ask the user to click the coffee cup
once or press their Raycast hotkey again. If it reports external ownership,
explain that **Restore Normal Sleep…** appears in the exceptional recovery menu
and requires a separate confirmation and administrator authorization because it
can override another tool.

## Shortcut behaves unexpectedly

Check Raycast Settings for duplicate commands using `⌥S`. Disable the retired
Script Command and bind only **Toggle Night Watch**. Do not use `killall
caffeinate`; unrelated processes are outside this extension's ownership.

## Raycast restart

Open the menu-bar command and refresh. A valid owned session should be recovered
from its guarded process and cache, while stale cache must be discarded. Never
create ownership based only on a PID file.

## Windows

### Cup shows paused

`powerSource="battery"` intentionally pauses protection. The utility never
changes DC values. Ask the user to connect AC power; do not modify battery
policy for them.

### Cup shows warning

Compare the active power scheme, AC standby value, journal presence, and tray
process from `preflight.ps1`. A changed scheme, Group Policy restriction,
damaged journal, or externally changed AC value is not an owned active state.
Ask the user to open **View Status** and use **Restore Original Power
Settings…** if offered. Do not run `powercfg` to force a result.

### App or watchdog exited

Run preflight. A missing journal means no recorded recovery remains. If the
journal is present, relaunch the official executable and let its recovery
prompt read and verify the recorded values. Never delete the journal before a
successful restore.

### Lock and display off is unavailable

The action is enabled only while Night Watch is active on AC power. It is
deliberately disabled while off, paused on battery, or in a warning state.

### Managed computer

If Windows reports a Group Policy override or access restriction, stop. Agent
Night Watch does not request administrator elevation or bypass organizational
policy.
