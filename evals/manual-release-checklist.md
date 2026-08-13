# Manual release checklist

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
