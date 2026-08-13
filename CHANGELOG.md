# Changelog

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
