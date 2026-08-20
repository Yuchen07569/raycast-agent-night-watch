# Agent Night Watch

[简体中文](README.zh-CN.md)

Agent Night Watch keeps long-running local agents working while a computer's
display is off. It has two deliberately small platform surfaces:

- macOS: a Raycast menu-bar coffee cup for closed-lid work.
- Windows: a native portable system-tray coffee cup for AC-powered closed-lid
  work and secure lock-and-display-off sessions.

Neither version uses telemetry, accounts, advertising, or background network
services.

## macOS Raycast extension

- Empty coffee cup: normal closed-lid sleep.
- Steaming coffee cup: Agent Night Watch is active.
- Click the coffee cup once to toggle directly in normal states.
- Enabling prompts for macOS administrator authorization every time.
- Disabling an owned session does not ask for authorization again.
- No timer, battery cutoff, password storage, privileged helper, or sudoers rule.

This differs from Raycast Coffee and other `caffeinate` wrappers: those prevent
idle sleep, while Agent Night Watch explicitly controls the global closed-lid
sleep override.

### Install

The Raycast Store submission is pending. Until it is approved, install from
source:

```sh
git clone https://github.com/Yuchen07569/raycast-agent-night-watch.git
cd raycast-agent-night-watch/raycast-extension
npm ci
npm run dev
```

Run **Night Watch Menu Bar** once to place the coffee cup in the menu bar. After
that, click the cup to toggle directly. Assign `⌥S` to **Toggle Night Watch** if
you also want the keyboard shortcut.

If macOS sleep is disabled by another tool or the extension is in a transition,
clicking the cup opens a recovery/status menu instead of overriding that state.

## Windows portable Beta

The Windows implementation is an invitation-only, unsigned Beta. It is a
single native Win32 executable with no Electron, .NET, Python, installer,
service, driver, or updater.

- Left-click the tray cup to toggle.
- Steaming cup: active on AC power.
- Paused cup: enabled but currently on battery; DC settings are untouched.
- Warning cup: the power plan, policy, or recovery journal needs attention.
- Right-click **Lock and Turn Off Displays** for a secured background session.

The Beta targets Windows 10 22H2 and Windows 11 on x64. A physical Windows lid
test is still required before closed-lid support is presented as generally
verified. Build details are in [`windows-tray/README.md`](windows-tray/README.md);
invited testers should follow the
[Chinese physical-device test guide](evals/windows-beta-test-guide.zh-CN.md).

## Install the Codex skill

Ask Codex:

> Use `$skill-installer` to install
> `https://github.com/Yuchen07569/raycast-agent-night-watch/tree/main/.agents/skills/agent-night-watch`.

The skill installs, checks, updates, and troubleshoots either platform. It never
enables or disables sleep on the user's behalf.

## Develop macOS

```sh
cd raycast-extension
npm ci
npm test
npm run lint
npm run build
```

See [SECURITY.md](SECURITY.md) before changing the power-control flow.

## Develop Windows

```powershell
cmake -S windows-tray -B out/windows-x64 -A x64 -DBUILD_TESTING=ON
cmake --build out/windows-x64 --config Release
ctest --test-dir out/windows-x64 -C Release --output-on-failure
```

## Safety

Closed-lid operation can increase heat. Keep laptops on a hard, ventilated
surface and turn Night Watch off when the job finishes. The Windows version
changes AC settings only; battery settings remain under Windows control. A real
closed-lid test is required before each platform release.

## License

MIT
