# Mounts: animal riding, then vehicles and bikes

Source: the OpenNV mounts design of 2026-10-25 (artifact "OpenNV Mounts and Vehicles"). This file is the multiplayer-facing version.

## What exists today

- **FalloutNV.exe** still carries Oblivion's horse riding as dead code. The procedure slots (Mount Horse, Dismount Horse, Clear Mount Position), the horse game settings and the condition functions (IsRidingHorse and friends) are all present, but the conditions return 0 and no package uses the procedures.
- **Oblivion.exe** has the working versions. The rider is parented to an `ActorParent` node on the horse mesh. Mounting lines the rider up at the horse's side, plays a paired animation, then attaches. Strafe keys turn the horse with a ramped turn rate (45 degrees per second cap, 80 per second ramp). Dismount puts the rider where the animation ends.
- **OpenMW and this fork** have no riding code. OpenMW uses Bullet as a collision world with its own kinematic movement solver, so there is no ready-made Bullet vehicle.
- **Pieces to reuse from openmw-fnv:** furniture sitting (enter, loop and exit animations, marker offsets), standing on moving objects, actor collision toggles, camera modes, Lua controls.

## Design

1. **Seat.** A rider follows a named node on the mount each frame, after the mount's own animation and movement. The rider's collision is off while seated and the mount carries collision.
2. **Driver.** Player input goes to the mount. Legged mounts (brahmin, nightstalkers, mod horses) reuse the creature's character controller and animations. Wheeled vehicles get a kinematic raycast-wheel solver on the collision world.
3. **Profiles.** Any base form can be a mount if a profile names it. Profiles are YAML files under `Data/OpenNV/mounts/`, keyed by plugin and form ID, with a matching Lua interface.
4. **Script surface.** IsRidingHorse and GetPlayerHasLastRiddenHorse become real. New Mount, Dismount, GetMount and GetRider functions, OnMount and OnDismount events, and `I.Mounts` in Lua.

## Multiplayer sync

- **The driver owns the mount.** Whoever sits in the driver seat becomes the authority for that mount's position, velocity and animation state, the same way TES3MP treats a player's own character. When nobody rides it, the cell authority owns it like any other actor.
- **New packet: `MountSeat`.** Fields: mount reference, seat index, rider GUID or none, action (mount, dismount, swap). The server checks the seat is free and broadcasts. A refusal sends the requester back to standing.
- **Movement.** A ridden legged mount is sent with the existing actor position and animation packets at the player's rate, not the slower NPC rate. Passengers are not sent at all, since they follow the seat node on every machine.
- **Vehicles.** The wheel solver is kinematic and deterministic, so the driver sends inputs plus a position correction a few times per second, and other machines run the same solver in between. This is why the plan prefers our own solver over Bullet dynamics.
- **NPC riders.** An NPC on a mount is owned by the cell authority, like the rest of its AI.
- **Saves.** The server stores who sits where and each player's last mount, with the player record.

## Milestones

| Step | Content | Needs |
|------|---------|-------|
| Mount 0 | Profile schema, loader, test plugin with a rideable brahmin and a simple bike | Nothing, can start now |
| Mount 1 | Player rides an animal: seat, legged driver, mounted camera, dismount with a room check, saves | Actor physics and animation layers in openmw-fnv |
| Mount 1-MP | `MountSeat` packet, driver authority, passengers follow the seat | Mount 1, plus the merge |
| Mount 2 | NPC riders, followers mount with the player, shooting from the saddle, theft rules | Package procedure sequencer |
| Mount 3 | Vehicles and bikes: wheel solver, crashes, passengers, fuel | Havok world collision shapes |
| Mount 4 | Mod documentation, converter notes for scripted riding mods | Mount 3 |

**What can start here first:** the `MountSeat` packet and server-side seat bookkeeping, tested by riding a Morrowind guar with a dummy seat node. That proves the sync model before FNV content exists.

## Open choices for Sen

- **Profile format:** YAML sidecar plus Lua (proposed, lets ESP-only mods work), or Lua only.
- **Vehicle physics:** our own kinematic wheel solver (proposed, deterministic for multiplayer), or Bullet dynamics with its raycast vehicle.
- **Steering keys:** Oblivion style, strafe keys steer and the mouse aims (proposed, lets you shoot while riding), or mouse steering like most modern games.
