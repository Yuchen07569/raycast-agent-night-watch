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

The privileged guard program is embedded into the authorization command before
the macOS password dialog appears. It is never loaded from a user-writable
script pathname. While privileged, it calls only the absolute system paths for
`pmset`, `grep`, and `sleep`.

The guard records the original `SleepDisabled` value in process memory, checks
the existence of a randomized manual stop signal once per second, and restores
the original value when it exits. It never creates, replaces, deletes, or
redirects output to files in the user-owned session directory. All cache and
log writes are performed by the unprivileged Raycast process. The UI reports
success only after reading `pmset -g` again.

## Local state

Owned-session state is stored with user-only permissions under:

```text
~/Library/Caches/com.yuchen.agent-night-watch
```

The cache never overrides `pmset -g` as the source of truth. A stale or invalid
cache is discarded rather than treated as permission to modify system state.
Session schema changes invalidate older ownership records instead of handing
their paths to a privileged process.

## External state

If `SleepDisabled=1` without a valid owned session, the extension reports that
another tool or leftover state owns the setting. Restoring normal sleep then
requires a separate destructive confirmation and administrator authorization.

## Reports

Open a private security advisory in the GitHub repository for vulnerabilities.
Do not include passwords, tokens, private logs, or other personal information.
