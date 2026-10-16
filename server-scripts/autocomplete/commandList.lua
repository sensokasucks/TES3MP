-- commandList.lua: tells each player's client which chat commands they can use, so the chat box
-- can suggest and autocomplete them as they type. Needs a client and server built with
-- ID_CHAT_COMMANDS (branch port/chat-autocomplete or later).
--
-- The list is built from:
--   1. the /help menus (scripts/menu/help.lua), which describe the built-in commands by rank
--   2. commands registered here with commandList.register(), for scripts that want a description
--   3. any other command registered with customCommandHooks, shown without a description
-- Each player only gets the commands their staff rank allows.
--
-- Install: put this file in server/scripts/custom/ and add
--   require("custom.commandList")
-- to server/scripts/customScripts.lua
--
-- Other scripts can describe their commands like this (rank 0 = everyone, 1 = moderator,
-- 2 = admin, 3 = server owner):
--   commandList.register("chest", "/chest", "Toggle private loot for containers", 0)

commandList = {}

-- Help menus to read, with the staff rank needed to see each one
commandList.helpMenus = {
    { "help player", 0 },
    { "help moderator page 1", 1 },
    { "help moderator page 2", 1 },
    { "help admin page 1", 2 },
    { "help admin page 2", 2 }
}

-- Commands that exist but aren't on the help menus, as { name, usage, description, rank }
commandList.registered = {
    { "kick", "/kick <pid>", "Kick a player", 1 },
    { "addadmin", "/addadmin <pid>", "Promote a player to admin", 3 },
    { "removeadmin", "/removeadmin <pid>", "Demote a player from admin", 3 }
}

-- Descriptions for commands other scripts register with customCommandHooks, used only when
-- that command is actually registered, as { name, usage, description }
commandList.hookDescriptions = {
    { "menu", "/menu", "Open the command menu" },
    { "chest", "/chest", "Toggle private loot for containers" },
    { "players", "/players", "List all players on the server" },
    { "greentext", "/greentext <text>", "Send a message in green text" },
    { "gt", "/gt <text>", "Send a message in green text" }
}

-- The staff rank each player's list was last built for, so a promotion or demotion sends a new one
commandList.sentRanks = {}

function commandList.register(name, usage, description, rank)
    for index, entry in ipairs(commandList.registered) do
        if entry[1] == name and entry[2] == usage then
            table.remove(commandList.registered, index)
            break
        end
    end

    table.insert(commandList.registered, { name, usage or ("/" .. name), description or "", rank or 0 })
end

local function stripColors(text)
    return (text:gsub("#%x%x%x%x%x%x", ""))
end

local function trim(text)
    return (text:gsub("^%s+", ""):gsub("%s+$", ""))
end

-- Turn a help menu's text into entries, reading "/command args" lines and the description after each
local function parseHelpText(text, rank, entries)
    local current = nil

    local function finish()
        if current == nil then return end

        local description = trim(current.description)
        local aliases = {}

        for alias in description:gmatch("%(/([^%s%)]+)%)") do
            table.insert(aliases, alias)
        end

        description = trim(description:gsub("%(/[^%s%)]+%)", ""))
        table.insert(entries, { current.name, current.usage, description, rank })

        local arguments = current.usage:match("^/%S+(.*)$") or ""

        for _, alias in ipairs(aliases) do
            table.insert(entries, { alias, "/" .. alias .. arguments, description, rank })
        end

        current = nil
    end

    for rawLine in (stripColors(text) .. "\n"):gmatch("([^\n]*)\n") do
        local line = trim(rawLine)

        local name = line:match("^/([%w_]+)")

        if name ~= nil then
            finish()
            current = { name = name:lower(), usage = line, description = "" }
        elseif current ~= nil and line ~= "" then
            current.description = current.description .. " " .. line
        end
    end

    finish()
end

function commandList.GetStaffRank(pid)
    local settings = Players[pid].data.settings
    return (settings ~= nil and settings.staffRank) or 0
end

-- Build the list of { name, usage, description } a player is allowed to use
function commandList.Build(pid)
    local rank = commandList.GetStaffRank(pid)
    local entries = {}

    if Menus ~= nil then
        for _, helpMenu in ipairs(commandList.helpMenus) do
            local menu = Menus[helpMenu[1]]

            if menu ~= nil and type(menu.text) == "string" then
                parseHelpText(menu.text, helpMenu[2], entries)
            end
        end
    end

    for _, entry in ipairs(commandList.registered) do
        table.insert(entries, entry)
    end

    local result = {}
    local names = {}
    local usages = {}

    for _, entry in ipairs(entries) do
        local name, usage, description, requiredRank = entry[1], entry[2], entry[3], entry[4]

        if rank >= requiredRank and usages[usage] == nil then
            table.insert(result, { name, usage, description })
            usages[usage] = true
            names[name] = true
        end
    end

    -- Commands from other scripts that nothing above describes
    if customCommandHooks ~= nil then
        local descriptions = {}

        for _, entry in ipairs(commandList.hookDescriptions) do
            descriptions[entry[1]] = entry
        end

        for name, _ in pairs(customCommandHooks.commands) do
            if names[name] == nil then
                local allowed = true
                local nameRequirement = customCommandHooks.nameRequirement[name]
                local rankRequirement = customCommandHooks.rankRequirement[name]

                if nameRequirement ~= nil then
                    allowed = tableHelper.containsValue(nameRequirement, Players[pid].accountName)
                elseif rankRequirement ~= nil then
                    allowed = rank >= rankRequirement
                end

                if allowed then
                    local described = descriptions[name]

                    if described ~= nil then
                        table.insert(result, { name, described[2], described[3] })
                    else
                        table.insert(result, { name, "/" .. name, "" })
                    end
                    names[name] = true
                end
            end
        end
    end

    return result
end

function commandList.Send(pid)
    if Players[pid] == nil or not Players[pid]:IsLoggedIn() then return end

    tes3mp.ClearChatCommands(pid)

    for _, command in ipairs(commandList.Build(pid)) do
        tes3mp.AddChatCommand(pid, command[1], command[2], command[3])
    end

    tes3mp.SendChatCommands(pid)
    commandList.sentRanks[pid] = commandList.GetStaffRank(pid)
end

customEventHooks.registerHandler("OnPlayerAuthentified", function(eventStatus, pid)
    commandList.Send(pid)
end)

-- Staff commands like /addmoderator change someone's rank; send that player a new list
customEventHooks.registerHandler("OnPlayerSendMessage", function(eventStatus, pid, message)
    for otherPid, player in pairs(Players) do
        if player ~= nil and player:IsLoggedIn() and commandList.sentRanks[otherPid] ~= nil and
            commandList.sentRanks[otherPid] ~= commandList.GetStaffRank(otherPid) then
            commandList.Send(otherPid)
        end
    end
end)

customEventHooks.registerHandler("OnPlayerDisconnect", function(eventStatus, pid)
    commandList.sentRanks[pid] = nil
end)

return commandList
