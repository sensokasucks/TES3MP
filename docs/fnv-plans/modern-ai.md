# Modern AI branch (Path 2)

Source: the OpenNV NPC AI plan of 2026-10-26 (artifact "OpenNV NPC AI Plan"). Path 1, matching the original FalloutNV.exe AI, is being built in openmw-fnv and is far along: detection, crime, followers, the combat planner with cover, grenades, combat items and weapon choice are merged. Path 2 is the optional modern AI and has not started. This file plans it for multiplayer.

## Shape

Both paths share one AI core:

- **World view** in: package records, combat styles, AI data, factions, game settings, cells, detection, quest state.
- **Brain** decides. The original brain is the default. The modern brain is a world setting, off unless chosen.
- **Intents** out: MoveTo, Follow, UseFurniture, PlayIdle, Say, Attack, Reload, TakeCover, Flee, UseDoor, Activate. Intents are small and serializable.
- **Executor** (C++) carries intents out: navmesh paths, avoidance, character physics, doors, furniture, turning, animation.

Multiplayer only has to replicate intents, their results and low-rate positions. Brain internals stay on the authority. That is why the modern brain is cheap to add to TES3MP once the seam exists.

## Safety rules

Script packages, packages with quest-stage conditions, Dialogue, Escort and Follow packages, "must complete" packages, and any actor in a running quest scene always use the original brain. The modern brain only replaces decisions that are free in vanilla: what to do in a sandbox, how to fight, how to move in a crowd.

## Phases

| Phase | Content | Needs (in openmw-fnv) |
|-------|---------|-----------------------|
| B0 Switch | World setting `ai backend = original or modern`, stored in the save. In multiplayer the server owns it, never per client. Switching converts state at the next package pick. | A0 AI seam and saved state |
| B1 Crowds | DetourCrowd steering as an avoidance mode, plus generated Recast navmesh where an FNV navmesh is missing or broken | A1 movement fidelity |
| B2 Daily life | Utility scoring inside Sandbox and Wander: needs (hunger, rest, social) that decay over game time, energy, time of day, ownership, who is nearby. Weather and gunfire reactions. | A2 packages, A3 off-screen life |
| B3 Tactics | More planner actions (suppressive fire, flanks, retreat to allies, heal allies), squad roles, shared last-known position, precomputed cover points. A per-base group brain after MGSV's command post: alert phase Battle, Search, Guard, back to routine. | A5 combat |
| B4 Companions | Formation following out of doorways and lines of fire, better waiting spots, stealth mirroring | A6 followers |
| B5 Civilians | Opt-in population layer that adds generic townsfolk from FalloutNV.esm's 769 generic NPC records | B1, B2, A3 abstract tier |

## Civilian population (B5) in short

- **Three tiers.** Realized: full model and AI, near and on screen. Rendered-lite: model and walk animation, AI once per second, no physics body. Abstract: a point on the navmesh with a schedule, no model.
- **Flow-field steering** for the lower tiers: one grid per town cell, one field per destination, computed once and cached. Each civilian reads a direction, adds a separation push, and steps at a fixed 10 Hz.
- **Regenerated from a seed.** A town's civilians come from (town, day, seed). A save stores only the seed, the deaths and a few flags.
- **Rough budget:** about 40 realized NPCs, 80 to 100 rendered, a few hundred abstract per worldspace. Goodsprings gets 15 to 20 more people, Freeside 60 to 100, the Strip 80 to 120.

## Multiplayer rules

- The cell authority runs brain and executor for actors in its cells. Other clients only receive.
- Detection scores every player and takes the best. Companions belong to the player who recruited them.
- When cell ownership moves, the saved AI state moves with it, so NPCs continue instead of restarting.
- Brains read no local-only data (camera, frame rate). Random is seeded per actor and the AI tick is fixed.
- An actor is in the full-update tier if it is in that tier for any player.
- Abstract and rendered-lite civilians are never sent. Every machine computes the same crowd from the seed and the town clock. Only realized civilians replicate.
- Heavy work that needs no scene can move to the server. See [server-ai.md](server-ai.md).

## Branch plan

- `fnv/ai-modern` in openmw-fnv for the single-player modern brain, since it must work with no server.
- `fnv-mp/ai-sync` here for the intent and result packets, the authority hand-over and the server workers.
- Both merge into `fnv-mp` once the merge branch exists.

**What can start here first:** the intent and result packet types, cell-authority hand-over carrying an opaque AI state blob, and the server worker framework. All three can be tested with Morrowind actors today.
