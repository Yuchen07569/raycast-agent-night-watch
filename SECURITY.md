# Security policy

## Power-control boundary

Agent Night Watch calls only these privileged system commands:

```text
/usr/bin/pmset -a disablesleep 1
/usr/bin/pmset -a disablesleep 0
```

Each enable operation runs a visible macOS administrator authorization dialog.
The password is handled by macOS and is never visible to, stored by, or sent
through this extension. The extension does not install a privileged helper,
daemon, launch agent, kernel extension, or sudoers rule.

The privileged guard records the original `SleepDisabled` value, checks for a
manual stop signal once per second, and restores the original value when it
exits. The UI reports success only after reading `pmset -g` again.

## Local state

Owned-session state is stored with user-only permissions under:

```text
~/Library/Caches/com.yuchen.agent-night-watch
```

The cache never overrides `pmset -g` as the source of truth. A stale or invalid
cache is discarded rather than treated as permission to modify system state.

## External state

If `SleepDisabled=1` without a valid owned session, the extension reports that
another tool or leftover state owns the setting. Restoring normal sleep then
requires a separate destructive confirmation and administrator authorization.

## Reports

Open a private security advisory in the GitHub repository for vulnerabilities.
Do not include passwords, tokens, private logs, or other personal information.
