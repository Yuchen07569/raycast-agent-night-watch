# Changelog

## [0.2.0-beta.1] - 2026-08-20

- Added a native, single-executable Windows x64 tray implementation independent
  of Raycast and external runtimes.
- Added AC-only automatic-sleep and lid-action transactions with read-back,
  external-change detection, rollback, and same-binary crash watchdog.
- Added locked display-off sessions, active/paused/warning tray states, Windows
  core tests, MSVC static analysis, Defender scanning, SHA-256 packaging, and an
  SPDX SBOM.
- Expanded the maintenance Skill and documentation for safe macOS/Windows
  platform routing.

## [0.1.5] - 2026-08-13

- Removed the user-writable privileged guard script and embedded the fixed
  guard program into the authorization command before the password dialog.
- Stopped all privileged writes to user-owned session paths; root now only
  reads a randomized stop signal and calls an explicit system-command allowlist.
- Replaced root-created readiness files with `pmset -g` plus verified launcher
  identity, and invalidated the previous session schema.
- Added adversarial regression tests for mutable paths, symlink writes, shell
  injection, command allowlisting, and embedded-shell syntax.

## [0.1.4] - 2026-08-13

- Changed normal menu-bar interaction to a direct single-click toggle without
  opening a second menu.
- Kept external ownership and transition states behind a safe recovery/status
  menu instead of overriding them.
- Removed two-second polling; hotkey and menu actions now redraw through the
  Raycast command lifecycle immediately after each completed operation.
- Limited duplicate-mount suppression to one second so a later real click
  always starts a new toggle operation.

## [0.1.3] - 2026-08-13

- Fixed a stale steaming menu-bar icon after `SleepDisabled` had already
  returned to `0`.
- Added a two-second live status calibration while Raycast keeps the menu-bar
  command loaded, with cleanup when the command unloads.

## [0.1.2] - 2026-08-13

- Fixed a stale toggle lock that could make both the hotkey and menu unable to
  disable an owned Night Watch session in Raycast's long-lived backend.
- Added short lock leases and ownership tokens so abandoned locks recover
  promptly without an older operation releasing a newer operation's lock.

## [0.1.1] - 2026-08-13

- Made the Skill preflight invocation portable across GitHub archive installs,
  which do not preserve executable file permissions.

## [0.1.0] - 2026-08-13

- Added the public Raycast extension source and install-maintenance Codex Skill.
- Added real `SleepDisabled` status, owned-session recovery, locking, and
  manual-only controls.
- Added bilingual documentation, CI, security policy, and rollback metadata.
