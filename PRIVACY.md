# Privacy

Agent Night Watch has no analytics, telemetry, accounts, advertising, automatic
updater, or network requests. The macOS extension reads local power state and
stores temporary session ownership data in the user's cache directory. The
Windows utility reads the active power scheme and power source and stores one
temporary recovery journal under `%LOCALAPPDATA%\AgentNightWatch`.

Administrator credentials remain inside the macOS authorization dialog and
are never received by the extension.

The Windows utility does not request administrator credentials. The lock and
display-off action uses Windows' own lock screen and never receives a password,
PIN, or Windows Hello data.
