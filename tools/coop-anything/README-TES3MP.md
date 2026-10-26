# Tools from coop-anything, set up for TES3MP

Copied from https://github.com/cranebeaker/coop-anything at commit a1ea6ed (MIT, see `LICENSE`). The folders
`chaos/`, `flight/` and `buildlock/` are unchanged copies; their own READMEs describe every option. This page only
says how to use them with TES3MP. All three need Python 3.8+ and nothing else.

## Bad network on purpose: `chaos/`

A UDP relay that sits between a TES3MP client and the server and adds delay, jitter, loss, loss bursts,
reordering, duplicates and outages, with a fixed seed so a run can be repeated. TES3MP needs no change for it.

1. Start the server as usual (UDP port 25565 by default, `port` in `tes3mp-server-default.cfg`).
2. Start the relay on the server PC. `--bind 0.0.0.0` lets the other PC reach it; the port is up to you:

   ```
   python -I tools/coop-anything/chaos/chaos.py relay --target 127.0.0.1:25565 --listen 25600 --bind 0.0.0.0 --profile bad_wifi --log chaos.jsonl
   ```

3. Point one client at the relay instead of the server: in its `tes3mp-client-default.cfg` set
   `destinationAddress` to the server PC and `port = 25600`. The other client keeps connecting directly, so you
   can compare a good and a bad link in the same session.
4. Change the network while you play (the relay prints its `rpc` port when it starts):

   ```
   python -I tools/coop-anything/chaos/chaos.py call --rpc-port <rpc> set_profile '{"profile":"hell"}'
   python -I tools/coop-anything/chaos/chaos.py call --rpc-port <rpc> partition '{"dir":"s2c","on":true,"duration_ms":4000}'
   ```

Profiles: `lan`, `good_wifi`, `bad_wifi` (the useful default for sync bugs), `mobile_tether`, `transatlantic`, `hell`.
Things worth watching on the bad client: NPCs and players moving and turning smoothly, an NPC in dialogue staying
put, cell authority changing hands when a player leaves, container and door state, deaths and kill counts.

## Packet flight recorder: `flight/`

The client and the server can record every packet they send and receive (`components/openmw-mp/FlightRecorder.hpp`).
It is off unless the environment variable `TES3MP_FLIGHT_DIR` names a folder:

```
set TES3MP_FLIGHT_DIR=C:\tes3mp-flight
set TES3MP_FLIGHT_SLOT=1
tes3mp.exe
```

Give each machine its own `TES3MP_FLIGHT_SLOT` (the server defaults to 0, clients to 1, so set 2 on the second
client). The newest 16 MB of traffic is kept in memory and written as a `.cofr` file when a client disconnects or
exits, and when the server exits. Then, with the files of all machines in one place:

```
python -I tools/coop-anything/flight/flight.py print server.cofr --limit 50 --schema tools/coop-anything/tes3mp-schema.json
python -I tools/coop-anything/flight/flight.py align server.cofr client1.cofr client2.cofr --schema tools/coop-anything/tes3mp-schema.json
python -I tools/coop-anything/flight/flight.py diverge server.cofr client1.cofr client2.cofr
```

`tes3mp-schema.json` names every packet ID (ID_PLAYER_POSITION instead of msg42). After adding a packet, regenerate it:
`python -I tools/coop-anything/make_tes3mp_schema.py deps/CrabNet/include/raknet/MessageIdentifiers.h`
(the path is wherever CrabNet's headers are on your machine).

The message type of every record is the TES3MP packet ID (the first byte of the packet, see
`components/openmw-mp/NetworkMessages.hpp`). Today the recorder logs traffic only; it does not take digests of the
world state yet, so `align` (sent and received side by side) is the useful view; `diverge` only finds traffic gaps until digests are added.

## Shared build folder: `buildlock/`

Several people and agents build in the same `build` folder. Two builds at once corrupt it. Run builds through the lock:

```
python -I tools/coop-anything/buildlock/buildlock.py run --tree build --owner <who> -- cmake --build build --target tes3mp
python -I tools/coop-anything/buildlock/buildlock.py status --tree build
```

The lock is the folder `build.buildlock` next to the build folder. A lock whose holder died is taken over
automatically.
