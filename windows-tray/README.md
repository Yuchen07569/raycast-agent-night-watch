# Agent Night Watch for Windows — Invitation Beta

This folder contains the native Windows system-tray version. It is independent
of Raycast and has no installer, external runtime, service, driver, telemetry,
network access, automatic updater, or default startup entry.

## Behavior

- Left-click the coffee cup to toggle AC Night Watch.
- On a laptop, AC automatic sleep and the AC lid action are temporarily set to
  Never / Do Nothing. DC values are never changed.
- On a desktop, only AC automatic sleep is changed.
- On battery, the cup shows paused and the original DC policy remains active.
- Right-click **Lock and Turn Off Displays** to secure the session while local
  work continues on AC power.
- Disable or exit to restore the exact recorded AC values.

The utility calls Windows PowrProf and power-request APIs directly. It does not
run `powercfg` or PowerShell and does not request elevation. Managed devices can
block writes through Group Policy; that state is reported and never bypassed.

## Build

Use Windows 10/11 with Visual Studio 2022 Build Tools and CMake:

```powershell
cmake -S windows-tray -B out/windows-x64 -A x64 -DBUILD_TESTING=ON
cmake --build out/windows-x64 --config Release --parallel
ctest --test-dir out/windows-x64 -C Release --output-on-failure
```

The executable is `out/windows-x64/Release/AgentNightWatch.exe`. MSVC uses the
static C++ runtime, so the executable does not require a separate Visual C++
runtime installation.

## Beta boundary

This build is unsigned and invitation-only. Verify the SHA-256 file from the
same GitHub Actions artifact or release before running it. Do not disable
SmartScreen, Defender, Smart App Control, or company policy.

Closed-lid behavior varies by firmware and sleep model. It is not considered
generally verified until the physical checklist in
[`evals/windows-beta-test-guide.zh-CN.md`](../evals/windows-beta-test-guide.zh-CN.md)
has passed on representative hardware.
