# Security policy

## macOS power-control boundary

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

## Windows power-control boundary

The Windows portable utility is an `asInvoker` native Win32 executable. It
does not request elevation or execute PowerShell, `powercfg`, a helper service,
or a downloaded binary. It calls the documented PowrProf and power-request APIs
directly.

An owned Windows session changes only two AC values in the active power scheme:

- automatic standby timeout becomes `0` (Never);
- lid close action becomes `0` (Do Nothing) only when Windows reports a lid.

DC/battery values are never written. The utility checks Group Policy access
before writing, journals the original AC values before the first change, reads
every value back, and rolls back the full transaction on failure. A
`PowerRequestSystemRequired` request is held only while an owned session is
active on AC power; it does not request that displays stay on.

The same executable launches a no-window watchdog tied to the tray process and
instance identifier. If the tray process exits unexpectedly, the watchdog
restores only values that still match the values Agent Night Watch wrote.
Externally changed values are reported and never overwritten. A checksum,
strict parser, atomic replace, file-size limit, and reparse-point checks protect
the recovery journal under `%LOCALAPPDATA%\AgentNightWatch`.

The Windows invitation Beta is unsigned. Release artifacts must come from
GitHub Actions, include SHA-256 and SPDX metadata, and remain labeled as Beta
until a physical lid-close test passes. Do not tell users to disable
SmartScreen, Defender, Smart App Control, or organizational policy.

## External state

If macOS `SleepDisabled=1` without a valid owned session, the extension reports that
another tool or leftover state owns the setting. Restoring normal sleep then
requires a separate destructive confirmation and administrator authorization.

## Reports

Open a private security advisory in the GitHub repository for vulnerabilities.
Do not include passwords, tokens, private logs, or other personal information.
