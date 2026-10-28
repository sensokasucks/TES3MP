# One launcher for single player and co-op

Sen's ask (2026-10-28): let players use the same launcher for single-player games too, so the work is not locked behind multiplayer.

## Where things stand

- **openmw-fnv launcher:** pull request #6 there adds a Game chooser (Morrowind or Fallout: New Vegas). New Vegas mode sets the content list (Morrowind.esm, then FalloutNV.esm), finds the Steam folders and remembers them. It launches the plain single-player `openmw` executable.
- **This fork:** the client is a separate executable, `tes3mp`. It always connects to a server at startup (from `--connect` or `destinationAddress` in `tes3mp-client-default.cfg`) and has no offline mode. The server list is a separate app, `tes3mp-browser`.

## Proposal

Keep two game executables built from the one merged tree, and put one launcher in front of them.

| Launcher button | Starts | Saves |
|-----------------|--------|-------|
| **Play** (single player) | `openmw` with the chosen game, as today | Normal local save files |
| **Host co-op** | `tes3mp-server` on this PC, then `tes3mp --connect=127.0.0.1` | On the server, under its data folder |
| **Join co-op** | `tes3mp --connect=<address>` from the server list or a typed address | On the host's server |

Why two executables and not one with a network switch:

- The single-player game stays exactly what openmw-fnv ships, with no multiplayer code paths in it. A multiplayer bug cannot break solo play.
- It needs no change to TES3MP's startup, which assumes a connection from the first frame.
- Sharing all FNV features is already handled by the merge: both executables link the same `openmw-lib` and the same FNV Lua scripts.

## Launcher work

1. **Mode tabs** on the openmw-fnv launcher's main page: Play, Host, Join. The Game chooser from pull request #6 applies to all three.
2. **Host tab:** server name, password, max players (default 4), port (default 25565), a "Start server" toggle that runs `tes3mp-server` hidden with a log window on request, and an "Allow over the internet" note pointing to Tailscale or ZeroTier instead of router port forwarding.
3. **Join tab:** the `tes3mp-browser` server list folded in as a page, plus favorites and a direct address box. The browser's ping and query code moves into a small shared library.
4. **Same content check:** before joining, compare the client's content list and plugin hashes with the server's, and offer to fix the list. TES3MP already sends the content list on connect, so this is a friendlier front end for it.
5. **Character handoff (later):** export a single-player character (SPECIAL, skills, perks, inventory, level) into a server player record, so a son can bring their solo character into co-op. One way only at first.
6. **Packaging:** one installer folder with `openmw`, `tes3mp`, `tes3mp-server` and the launcher, all from the same commit, so versions never mismatch.

## Order

1. Merge pull request #6 in openmw-fnv once it builds on Sen's PC.
2. Add the Play, Host and Join tabs in openmw-fnv's launcher, with Host and Join greyed out when the tes3mp executables are not next to it. That way the single-player build ships on its own today.
3. After the merge, the `fnv-mp` build places all executables together and the tabs light up.
4. Character handoff once the Lua global relay exists.
