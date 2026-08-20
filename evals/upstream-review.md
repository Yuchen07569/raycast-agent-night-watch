# Upstream review

Audit dates: 2026-08-13 and 2026-08-20. Upstream text and code were treated as untrusted and
inspected statically. No upstream scripts were executed and no code was copied.

| Source | Inspected commit | License | Decision |
|---|---|---|---|
| [Raycast Coffee](https://github.com/raycast/extensions/tree/main/extensions/coffee) | `0e62f7d1cffac7bab0d06caf87c5e85507989804` | MIT | Reject as backend; it uses `caffeinate`, detects processes broadly, and stops them with `killall`. Retain only the general menu-bar status pattern. |
| [Sleepless](https://github.com/Aboudjem/Sleepless) | `2a690e50724ffc17440ef58e5e0c9f69c82452fa` | MIT | Do not adopt; it is a standalone app with timers, battery policy, and a sudoers grant. Retain read-back and threat-model ideas. |
| [Capsomnia](https://github.com/fuji-mak/Capsomnia) | `2ca7c85c0317872595ad89abf7a386910a59d953` | MIT | Do not adopt; it is a standalone app with a helper and Caps Lock control. Retain visible-state and recovery ideas. |
| [MacClosedAwake Skill](https://github.com/onezion12344/mac-closed-awake) | `ed0b049205f057df9530c059a20b1bc2419d4876` | No recognized repository license | Reject; it requires a hard-coded local project, Bun, MCP, and a privileged helper. |
| [Don't Stop Skill](https://github.com/aannuuj/dont-stop) | `8d2925eef7f4eea8b5b8f7a44b5846e08229fa5d` | No recognized repository license | Reject as dependency; it requires its own app/helper and supports automatic task wrapping. Retain explicit ownership and recovery guidance. |
| [onezion-caffeinate](https://github.com/onezion12344/onezion-skills) | `5f8ff504b6d45a424257416d3055619e570b7234` | No recognized repository license | Reject; it directly runs broad shell commands and lacks Raycast state ownership. |
| [PowerToys Awake](https://github.com/microsoft/PowerToys) | `ddc536c69668837470439ecce68b7ce1b2094175` | MIT | Do not depend on it; its documented user-session request does not cover the Windows lock screen or explicit lid sleep. Retain the system-request lifecycle pattern. |
| [NoSleep](https://github.com/CHerSun/NoSleep) | `81cc6e4c85db9b9323def8eebc6cdbc14d3582aa` | Unlicense | Do not copy; it resets idle state frequently and does not provide transactional lid-setting ownership. Retain only evidence that a minimal tray surface is viable. |
| [Insomnia](https://github.com/stanley-projects/Insomnia) | `971055eeeeb78ca6873dcc6d1a5eee3918b6774a` | MIT | Do not copy or adopt Electron/agent monitoring. It covers AI-aware idle prevention but not the locked-session and AC lid transaction promised here. |
| [keepawake](https://github.com/cesarferreira/keepawake) | `5b3016a51a77dea774421d1f3e8ccfef062ecb30` | MIT | Do not copy. Its multi-platform timers and scheduling are outside the manual-only scope. |

## Adopted design constraints

- Read `pmset -g` as system truth.
- Verify state after every write before reporting success.
- Distinguish owned state from another tool's state.
- Keep privileges narrow, visible, and reversible.
- Require physical closed-lid testing before release claims.
- On Windows, write AC values only and pause the system request on battery.
- Restore only owned values; never overwrite a later external change.

## Rejected capabilities

- Timed or automatic activation.
- Battery or thermal policy presented as a substitute for manual control.
- Passwordless sudo, privileged helpers, MCP servers, or launch agents.
- Killing unrelated `caffeinate` processes.
- Direct sleep writes from the Codex Skill.
