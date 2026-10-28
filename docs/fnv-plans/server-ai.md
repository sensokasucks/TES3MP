# Server-side AI work

Sen's ask (2026-10-28): let the game server handle some AI grunt work.

## What the tes3mp-server is today

The tes3mp-server is a relay with Lua scripting. It tracks players, cells, objects and actor lists, routes packets, and runs the server CoreScripts. It does not load game content into a world. It has no physics, no navmesh, no animation and no rendering. In each cell one client is the authority and runs the real AI for that cell's actors.

So the server cannot run the realized-tier AI as it stands. That needs collision, animation root motion and the navmesh, which live in the client engine. Moving that onto the server would mean building a headless game world into it, which is a large job.

## The split

The good news is that much of the AI cost in the plans is not scene work. It is bookkeeping, scheduling and search over data. That can run on the server, which is usually the least loaded machine and the one that is always on.

| Work | Runs on | Why |
|------|---------|-----|
| Realized NPC brain and executor (walking, shooting, cover, doors) | Cell authority client | Needs physics, animation and collision |
| Off-screen life (A3 low process): unloaded NPCs walking their schedules, sleeping, eating | **Server** | Needs only the navmesh graph, package data and the game clock |
| Abstract civilians (B5): schedules, needs, positions on the navmesh | **Server** | Pure data. The server sends a civilian's state only when it is promoted to realized |
| Flow-field building for crowds | **Server**, cached on disk | A one-off grid search per town cell and destination |
| Group brains for faction bases (B3 command post): alert phase, role hand-out, reinforcements | **Server** | Decisions over the group, not motion. Spans cells, so no single client is the natural owner |
| Shared last-known position and squad knowledge | **Server** | It sees every player |
| Detection scores across several players | Cell authority client | Needs line-of-sight raycasts |
| Cover point precompute per cell | **Server** or offline tool, cached | Data search, done once |
| Quest flags, global variables, the Lua global state relay | **Server** | The one place that must stay consistent |
| Long cross-cell path searches | **Server** worker threads | The NAVM graph is data. The client only gets the resulting waypoints |

## What the server needs for this

1. **Load FNV data headless.** Read FalloutNV.esm records the AI needs (NAVM, PACK, CSTY, NPC_ AI data, FACT, GMST) through OpenMW's `components/esm4`. The server already links `components`, so this is a loader, not a new dependency. No meshes, no textures.
2. **Navmesh graph.** Build the NAVM triangle graph per cell, with door links between cells. Path search on worker threads.
3. **Worker framework.** A job queue on a thread pool, with results sent as packets on the next server tick. Lua can queue jobs and receive results.
4. **Same code as single player.** The off-screen simulation and the civilian layer are written once in a shared library in `components`, used by the single-player client and by the server. In single player the client runs them itself. In multiplayer the server does and the clients skip them.
5. **Hand-over packets.** Promote: the server tells the cell authority "civilian or NPC X enters your scene at this position, in this package, with this AI state". Demote: the authority hands the state back when the actor leaves the scene.

## Steps

| Step | Content | Size |
|------|---------|------|
| S0 | Worker thread pool and job packets on the server, with a Lua API. Test with a dummy job. | Small |
| S1 | Headless ESM4 record loader on the server for the AI records listed above | Medium |
| S2 | NAVM graph and server-side path search, results sent to the requesting client | Medium |
| S3 | Shared off-screen simulation library, run by the server for unloaded cells, with promote and demote hand-over | Large |
| S4 | Server-run abstract civilians and flow fields (needs B5 design work in openmw-fnv) | Medium |
| S5 | Server group brains for faction bases | Medium |

## Later option: a headless AI host

If the cell authority client turns out to be the bottleneck, for example when one son's laptop owns a busy cell, the next step is a headless OpenMW client with rendering off. It would join the server as a special player that takes authority for every cell no human is standing in, or for all cells. That reuses the whole client engine and needs no server rewrite. It is worth doing only after measuring a real problem.
