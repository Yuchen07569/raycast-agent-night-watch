# Manual release checklist

## macOS

- [ ] `⌥S` enable shows one administrator authorization dialog.
- [ ] Authorization cancellation leaves `SleepDisabled=0` and no live session.
- [ ] Owned enable results in `SleepDisabled=1` and a steaming cup.
- [ ] `⌥S` disable restores `SleepDisabled=0` within two seconds.
- [ ] Menu actions produce the same results as the shortcut.
- [ ] Rapid repeated activation creates one authorization flow and one session.
- [ ] Raycast restart recovers the owned session and correct icon.
- [ ] External `SleepDisabled=1` displays external ownership and a separately
      confirmed recovery action.
- [ ] Empty and steaming cups are legible on light and dark menu bars.
- [ ] A heartbeat continues through a representative physical lid-close test.
- [ ] Normal closed-lid sleep returns after Night Watch is disabled.
- [ ] A clean source install and, when available, Store install both work.

## Windows invitation Beta

- [ ] GitHub Actions x64 Release build, CTest, MSVC analysis, Defender scan,
      SHA-256 package, and SPDX generation pass.
- [ ] The unsigned Beta is shown only to invited testers with a SmartScreen
      warning; no instruction asks them to weaken Windows security.
- [ ] Left-click toggles off → active → off without opening a window.
- [ ] Right-click menu exposes status, lock-and-display-off, recovery, and exit.
- [ ] A desktop changes AC standby only and never touches the lid setting.
- [ ] A laptop changes AC standby and AC lid action, then restores both exact
      original values on disable and exit.
- [ ] DC standby and lid values remain byte-for-byte unchanged.
- [ ] Battery power shows paused and uses the original DC policy; reconnecting
      AC returns to active.
- [ ] Lock and display off requires Windows Hello/password on return while a
      ten-minute heartbeat continues.
- [ ] Killing the tray process makes the watchdog restore the exact AC values.
- [ ] Changing the active plan or edited values produces warning state without
      overwriting the external change.
- [ ] Two physical Windows laptops pass ten-minute AC lid-close heartbeats,
      including one Modern Standby device and one S3 device where available.
- [ ] A lid-close test that disconnects AC confirms the original battery policy
      takes effect.
- [ ] One physical Windows desktop passes the secured display-off heartbeat.
- [ ] CPU, private working set, executable size, Defender/SmartScreen result,
      OS version, hardware, and sleep model are recorded.
