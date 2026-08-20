# Security and provenance

Agent Night Watch is maintained at:

`https://github.com/Yuchen07569/raycast-agent-night-watch`

The macOS extension uses visible administrator authorization per enable. It
does not store the password or install passwordless privileges. The bundled
guard is readable shell source and calls only system `pmset` for the closed-lid
sleep override.

The public implementation was informed by, but does not copy code from,
Raycast Coffee, Sleepless, Capsomnia, MacClosedAwake, Don't Stop, and
onezion-caffeinate. Exact inspected commits and adoption decisions are recorded
in the repository's `evals/upstream-review.md`.

The Windows utility is native Win32 code built from the same public repository.
It calls `PowerCreateRequest`, `PowerSetRequest`, and the documented PowrProf
APIs directly. It does not execute PowerShell or `powercfg`, request elevation,
install a service or driver, touch DC/battery policy, or communicate over the
network. A same-binary watchdog restores the journaled AC settings if the tray
process exits unexpectedly. Group Policy and external setting changes are
reported rather than bypassed or overwritten.

Closed-lid operation has heat risk on both platforms. Windows Beta changes only
AC values and pauses on battery. Never present either platform as a guarantee
that an agent, network, power supply, or machine will remain healthy.
