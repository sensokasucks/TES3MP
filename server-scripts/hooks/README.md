# Gameplay event hooks

Branch `port/hooks` (off `port/openmw-0.51`, which now includes menu-hotkey and chat-autocomplete).
Adds a new packet (`ID_PLAYER_MENU`), so the client and server must both be built from this branch.

## What the engine adds
New server Lua events (global functions, called only if defined):

| Event | When |
|---|---|
| `OnPlayerAttack(pid)` | an attack by `pid` resolves against a target (hit or miss) |
| `OnPlayerCast(pid)` | `pid` finishes casting a spell (success or fail) or uses an enchanted item |
| `OnPlayerHealthThreshold(pid, threshold, falling)` | `pid`'s health crosses a fraction added with `AddHealthThreshold` |
| `OnPlayerMenu(pid, menu)` | `pid` opens, switches or closes a menu (`""` = closed) |

New functions:
- Attack details: `GetAttackTargetIsPlayer/Pid/RefId/RefNum/MpNum`, `GetAttackType` (0 melee, 1 ranged), `GetAttackSuccess`,
  `GetAttackDamage`, `GetAttackBlocked`, `GetAttackKnockdown`, `GetAttackStrength`, `GetAttackRangedWeaponId/AmmoId`.
- Cast details: `GetCastTargetIsPlayer/Pid/RefId/RefNum/MpNum`, `GetCastType` (0 spell, 1 item), `GetCastSuccess`,
  `GetCastSpellId`, `GetCastItemId`.
- `AddHealthThreshold(fraction)`, `ClearHealthThresholds()`. No thresholds means the event never fires.
- `GetPlayerMenu(pid)`, `IsPlayerInMenu(pid)`, `GetPlayerIdleTime(pid)` (seconds since the last movement, attack, cast,
  menu change or chat message).

Menu names: inventory, container, companion, mainmenu, journal, scroll, book, alchemy, repair, dialogue, barter, rest,
spellbuying, travel, spellcreation, enchanting, recharge, training, merchantrepair, levelup, chargen, loading, jail,
quickkeys, console, other.

## Server scripts in this folder
Copy to `server/scripts/custom/` and add to `server/scripts/customScripts.lua`, in this order:
```lua
require("custom.gameplayHooks")   -- required: routes the events into customEventHooks
require("custom.menuProtection")  -- optional: no PvP damage while in dialogue, chargen, loading or jail
require("custom.hooksDemo")       -- for testing: logs every event as [hooks] ... and adds /idle
require("custom.luaEvents")       -- event channel to client Lua mods; adds /luaping
require("custom.sharedStorage")   -- shared storage sections for client Lua mods (needs luaEvents)
require("custom.modSync")         -- per-mod sync settings from data/custom/modSync.json
```
Other scripts then use `customEventHooks.registerHandler("OnPlayerAttack", function(eventStatus, pid) ... end)` etc.

## Testing checklist
- Hit an NPC and another player, with melee and a bow: `[hooks] X attacked Y: hit, damage N` in the server log.
- Cast a spell, fail a spell, use an enchanted item: `[hooks] X cast ...`.
- Drop below half health, then a quarter, then heal: `[hooks] X health went below 50%` etc.
- Open inventory, talk to an NPC, open the console: `[hooks] X menu: ...`, then `(closed)`.
- `/idle` shows each player's idle seconds and open menu.
- With menuProtection: hit a player who is talking to an NPC; their health and fatigue come back half a second later.

## Lua event channel (client Lua <-> server Lua)
Engine: packet `ID_PLAYER_LUA_EVENT` (name up to 64 chars, payload up to 16 KB, 30 events/s per player each way).
- Server: `tes3mp.SendLuaEvent(pid, name, payload)` and the event `OnPlayerLuaEvent(pid, name, payload)`.
- Client: package `openmw.multiplayer` for player scripts (`isMultiplayer()`, `sendToServer(name, payload)`, `receive()`),
  wrapped by the built-in player script `scripts/tes3mp/multiplayer.lua`, which handles JSON.

Server scripts use [luaEvents.lua](luaEvents.lua):
```lua
local luaEvents = require("custom.luaEvents")
luaEvents.toast(pid, "Welcome!")                        -- message box on the player's screen
luaEvents.send(pid, "questMarker", { x = 1, y = 2 })    -- any table, encoded as JSON
luaEvents.register("myModAction", function(pid, data) ... end)  -- only registered names are accepted
```
Client mods (OpenMW 0.51 Lua, PLAYER scripts):
```lua
local I = require('openmw.interfaces')
return {
    eventHandlers = { MP_questMarker = function(data) print(data.x, data.y) end },  -- server event "questMarker"
    engineHandlers = { onKeyPress = function(key) I.Multiplayer.send('myModAction', { key = key.code }) end },
}
```
Built-in client events: `toast` shows a message box; `ping` answers with `pong`.

Test: install luaEvents.lua, then type `/luaping` in chat. You should see "Hello from the server's Lua!" in a message box
and "Client Lua answered in N ms." in chat.

## Mod compatibility
**Lua mods in the required list.** Clients now report their Lua mods (`.omwscripts` content files) along with their plugins.
If `requiredDataFiles.json` names no `.omwscripts` files, the server ignores them, so existing lists keep working. If it
names at least one, Lua mods are checked like plugins: same files, same order, matching checksum (or `[]` for any). The
connection error then lists the Lua mods too. The checksum only covers the `.omwscripts` file, not the scripts it lists.

**Shared storage** ([sharedStorage.lua](sharedStorage.lua)). Client mods call `I.Multiplayer.sharedSection('MyMod')`
and get `get`, `set`, `asTable`, `isReady` and `subscribe`. Every player sees the same values, the server saves them to
`data/custom/sharedStorage.json` every 30 seconds and on shutdown, and a write appears in `get()` once the server has
accepted it. Server scripts can read and write sections, lock one to a staff rank with `protect`, or check writes with
`registerValidator`. A whole section has to fit in one 16 KB event when a player first subscribes. Table values that
contain a number field named `n` come back as lists (a dkjson quirk).

**Per-mod sync settings** ([modSync.lua](modSync.lua)). Instead of editing config.lua and clientVariableScopes.lua for
each mod, list its synchronized mwscripts and globals in `data/custom/modSync.json` (format at the top of modSync.lua).
An entry only applies when its plugin is in `requiredDataFiles.json`.
