# OpenNV plans for the TES3MP fork

These plans move the multiplayer-side work for OpenNV (Fallout: New Vegas on OpenMW) into this repository, so it can start before the FNV fork and this TES3MP port are merged.

| Plan | What it covers |
|------|----------------|
| [mounts.md](mounts.md) | Animal riding, then vehicles and bikes, as an engine feature with multiplayer sync built in |
| [modern-ai.md](modern-ai.md) | Path 2, the optional modern NPC AI: crowds, utility daily life, squad tactics, civilian population |
| [physics-gore.md](physics-gore.md) | Collision, ragdolls, scaling to many ragdolls and AIs (GPU skinning, ragdoll sleep, VAT crowds), generic gore that also works in Morrowind, and bleeding (damage over time plus blood trails and pools) |
| [server-ai.md](server-ai.md) | Which AI work the tes3mp-server can take off the players' PCs, and which must stay on a client |
| [launcher.md](launcher.md) | One launcher for single player and co-op, so nothing is locked behind multiplayer |

## Where things live

- **FNV engine fork:** github.com/sensokasucks/openmw-fnv. The integration branch is `claude/project-thread-mvzwbm`. All single-player FNV work (packages, combat, Pip-Boy, gore, effects) lands there first.
- **This fork:** github.com/sensokasucks/TES3MP, branch `port/openmw-0.51`. TES3MP ported to OpenMW 0.51, on the OpenMW master of 2026-04-29.
- **Merge plan:** bring this port up to the same OpenMW base as openmw-fnv, then merge both into one `fnv-mp` branch. The trial merge on 2026-10-25 had only 12 FNV-caused conflicts. The rest came from the two trees being on different OpenMW snapshots.

## Rules that every plan here follows

1. **Single player first.** Every feature must work in plain single player with no server. Multiplayer is a layer on top, never a requirement.
2. **Original behavior is the default.** Modern AI, crowds and mounts are opt-in settings. Quests always run on the original rules.
3. **One authority per thing.** One machine decides each piece of state (a cell's NPCs, a mount, a quest flag) and the others mirror it. That keeps three players on a home connection cheap.
4. **Deterministic where possible.** Seeded random per actor and fixed ticks, so machines can compute the same result instead of sending it.
5. **Lua global state needs a relay.** TES3MP has no hooks in mwlua today, and most FNV game state (quests, SPECIAL, effects, AI packages) lives in Lua globals. A generic authority plus Lua event relay layer is the first multiplayer task after the merge, and several plans below depend on it.

## What can start in this repo now

These pieces do not need the FNV code to be merged first:

- The generic Lua global event relay (packet type, server routing, client hook in mwlua).
- The server-side AI worker framework in server-ai.md, tested with Morrowind actors.
- The mount seat sync packet and its server handling, tested with a Morrowind creature.
- Launcher modes (Solo, Host, Join) in the TES3MP launcher and browser.
