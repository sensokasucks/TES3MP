# Chat command autocomplete

Client branch: `port/chat-autocomplete` (off port/openmw-0.51 07f8f62). Adds packet `ID_CHAT_COMMANDS`, so the
client and server must both be rebuilt from this branch.

## Install the server script
1. Copy `commandList.lua` to `server/scripts/custom/`.
2. Add `require("custom.commandList")` to `server/scripts/customScripts.lua`.

The script sends each player, when they log in, the commands their staff rank allows. It reads the built-in
commands from the `/help` menus, adds a few that aren't there (`/kick`, `/addadmin`, `/removeadmin`), and picks up
anything other scripts registered with `customCommandHooks` (`/menu`, `/chest`, ...). Scripts can give their
commands a usage line and description with `commandList.register(name, usage, description, rank)`.
After a rank change (e.g. `/addmoderator`), that player gets a new list the next time anyone chats.

## Using it in game
- Type `/` and matching commands appear above the chat line, with usage and a short description.
- Tab completes the highlighted command; press Tab again to cycle (Shift+Tab goes back).
- Up/Down move the highlight while names are suggested; otherwise they browse chat history as before.
- After the command name and a space, the box shows how that command is used.

## Lua API (server)
`tes3mp.ClearChatCommands(pid)`, `tes3mp.AddChatCommand(pid, name, usage, description)`, `tes3mp.SendChatCommands(pid)`.
