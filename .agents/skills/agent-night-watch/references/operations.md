# Operations

## Install

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

## Update

- Store installs update through Raycast. Verify the listing and current status;
  do not replace them with a source build unless the user explicitly asks.
- Source installs update only from a named release tag. Preserve the package
  name and command names so Raycast can retain settings where possible.
- Confirm `SleepDisabled=0` before replacing or reimporting the extension.

## Uninstall

1. Ask the user to disable Agent Night Watch manually.
2. Verify `SleepDisabled=0` with `pmset -g`.
3. If it remains `1`, stop and use the troubleshooting workflow. Do not force
   the system setting from this Skill.
4. Ask the user to remove the extension from Raycast Settings.
5. Remove source files only when the exact path is confirmed and the user has
   explicitly requested deletion. Never delete the session cache while system
   sleep is disabled.
