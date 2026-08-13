# Agent Night Watch

[简体中文](README.zh-CN.md)

Agent Night Watch is a Raycast menu-bar switch that keeps long-running local
agents working when a MacBook lid is closed. It uses macOS `SleepDisabled` as
the source of truth instead of a normal idle-sleep assertion.

- Empty coffee cup: normal closed-lid sleep.
- Steaming coffee cup: Agent Night Watch is active.
- Click the coffee cup once to toggle directly in normal states.
- Enabling prompts for macOS administrator authorization every time.
- Disabling an owned session does not ask for authorization again.
- No timer, battery cutoff, password storage, privileged helper, or sudoers rule.

This differs from Raycast Coffee and other `caffeinate` wrappers: those prevent
idle sleep, while Agent Night Watch explicitly controls the global closed-lid
sleep override.

## Install the Raycast extension

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

## Install the Codex skill

Ask Codex:

> Use `$skill-installer` to install
> `https://github.com/Yuchen07569/raycast-agent-night-watch/tree/main/.agents/skills/agent-night-watch`.

The skill installs, checks, updates, and troubleshoots the Raycast extension. It
never enables or disables sleep on the user's behalf.

## Develop

```sh
cd raycast-extension
npm ci
npm test
npm run lint
npm run build
```

See [SECURITY.md](SECURITY.md) before changing the power-control flow.

## Safety

Closed-lid operation can increase heat and battery use. Keep the MacBook on a
hard, ventilated surface and turn Night Watch off when the job finishes. A real
closed-lid test is required before each release because behavior may vary by
hardware and macOS version.

## License

MIT
