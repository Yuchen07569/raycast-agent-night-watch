# Operations

Choose the section matching the preflight's `platform` value.

## macOS Raycast extension

### Install

1. Run `/bin/sh scripts/preflight.sh` from the Skill directory and require
   macOS plus Raycast.
2. Prefer the official Raycast Store listing after the repository README links
   to it.
3. Before Store approval, install the tagged source release:
   - Clone `https://github.com/Yuchen07569/raycast-agent-night-watch.git`.
   - Check out the requested release tag, never an unreviewed fork.
   - Run `npm ci`, `npm test`, `npm run lint`, and `npm run build` inside
     `raycast-extension/`.
   - Run `npm run dev` to import it into Raycast.
4. Ask the user to enable **Night Watch Menu Bar** and optionally assign `⌥S`
   to **Toggle Night Watch**.
5. Do not activate Night Watch as part of installation.

### Update

- Store installs update through Raycast. Verify the listing and current status;
  do not replace them with a source build unless the user explicitly asks.
- Source installs update only from a named release tag. Preserve the package
  name and command names so Raycast can retain settings where possible.
- Confirm `SleepDisabled=0` before replacing or reimporting the extension.

### Uninstall

1. Ask the user to disable Agent Night Watch manually.
2. Verify `SleepDisabled=0` with `pmset -g`.
3. If it remains `1`, stop and use the troubleshooting workflow. Do not force
   the system setting from this Skill.
4. Ask the user to remove the extension from Raycast Settings.
5. Remove source files only when the exact path is confirmed and the user has
   explicitly requested deletion. Never delete the session cache while system
   sleep is disabled.

## Windows portable utility

### Install

1. Run `scripts/preflight.ps1` and require Windows 10 22H2 or Windows 11 on
   x64. ARM64 remains unsupported until a physical release test is recorded.
2. Before Microsoft Store publication, use only a named invitation Beta from
   `https://github.com/Yuchen07569/raycast-agent-night-watch/releases`.
3. Download the ZIP and its `.sha256` file from the same release. Verify the
   SHA-256 before extracting; stop on any mismatch.
4. Explain that an unsigned invitation Beta can trigger SmartScreen. Do not
   weaken SmartScreen, Defender, Smart App Control, or enterprise policy.
5. Extract `AgentNightWatch.exe` to a user-chosen folder and launch it. Do not
   add startup entries or enable Night Watch during installation.

### Update

- Ask the user to left-click the steaming cup to disable Night Watch, then run
  preflight. Require no running `AgentNightWatch` process and no recovery
  journal before replacing the executable.
- Update only from a named maintainer release with a matching checksum. The
  portable Beta has no background updater.

### Uninstall

1. Ask the user to disable Night Watch and choose **Exit** from the tray menu.
2. Run preflight. Require `processRunning=false` and
   `sessionJournal="missing"` before deletion.
3. If a journal remains, stop and use Windows troubleshooting. Never delete it
   to make the status look clean.
4. Delete only the exact portable executable or extracted folder confirmed by
   the user. Do not alter Windows power settings directly.
