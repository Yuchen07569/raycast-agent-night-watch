# Troubleshooting

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
