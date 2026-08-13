# Adversarial review

## Against adoption

- A Skill about sleep control could accidentally broaden into privileged system
  writes or automated activation.
- Source installation introduces network and dependency risk before Store
  approval.
- Cache state could be mistaken for authoritative power state.

## For adoption

- The Skill is explicit-only and repeatedly prohibits write operations.
- Its only executable is a read-only preflight script with no network access.
- Installation is pinned to this repository and named release tags; Store is
  preferred after approval.
- `SleepDisabled` from `pmset -g` remains authoritative and ownership is stated
  separately.

## Decision

Adopt for public release. The Skill adds install and troubleshooting guidance
without gaining the ability to operate the user's sleep switch. Rollback is
independent removal of the installed Skill directory; the Raycast extension
continues to work manually.
