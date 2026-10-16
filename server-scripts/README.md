# Server scripts for this port

CoreScripts custom scripts that go with engine features on this branch. Copy each folder's `.lua` files to
`server/scripts/custom/` and require them from `server/scripts/customScripts.lua`; each folder's README has the details.

- `hooks/`: gameplay events, Lua event channel, shared storage, mod sync settings
- `autocomplete/`: chat command list for autocomplete
- `menu/`: `/menu` GUI menu (F7)
