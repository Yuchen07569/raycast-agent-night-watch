# Security and provenance

Agent Night Watch is maintained at:

`https://github.com/Yuchen07569/raycast-agent-night-watch`

The extension uses visible macOS administrator authorization per enable. It
does not store the password or install passwordless privileges. The bundled
guard is readable shell source and calls only system `pmset` for the closed-lid
sleep override.

The public implementation was informed by, but does not copy code from,
Raycast Coffee, Sleepless, Capsomnia, MacClosedAwake, Don't Stop, and
onezion-caffeinate. Exact inspected commits and adoption decisions are recorded
in the repository's `evals/upstream-review.md`.

Closed-lid operation has heat and battery risk. Never present the feature as a
guarantee that an agent, network, or machine will remain healthy.
