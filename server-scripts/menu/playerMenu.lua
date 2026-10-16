-- playerMenu.lua: a clickable menu for common chat commands.
-- Open it with /menu (or the client's menu hotkey, which sends /menu).
-- Every action runs the matching chat command, so the usual rank checks still apply;
-- staff-only entries are also hidden from players who can't use them.
-- Install: put this file in server/scripts/custom/ and add
--   require("custom.playerMenu")
-- to server/scripts/customScripts.lua

local playerMenu = {}

-- Creatures offered in the Disguise list, as { "Shown name", "refId" }.
-- Add your own (plugin creatures work too); "Type a creature ID..." covers anything else.
playerMenu.disguises = {
    { "Mudcrab", "mudcrab" }, { "Rat", "rat" }, { "Scrib", "scrib" }, { "Cliff Racer", "cliff racer" },
    { "Guar", "guar" }, { "Kagouti", "kagouti" }, { "Alit", "alit" }, { "Nix-Hound", "nix-hound" },
    { "Netch (bull)", "netch_bull" }, { "Netch (betty)", "netch_betty" }, { "Kwama Forager", "kwama forager" },
    { "Slaughterfish", "slaughterfish" }, { "Dreugh", "dreugh" },
    { "Skeleton", "skeleton" }, { "Ancestor Ghost", "ancestor_ghost" }, { "Bonewalker", "bonewalker" },
    { "Ash Slave", "ash_slave" }, { "Ash Zombie", "ash_zombie" }, { "Corprus Stalker", "corprus_stalker" },
    { "Dwarven Ghost", "dwarven ghost" }, { "Centurion Spider", "centurion_spider" },
    { "Centurion Sphere", "centurion_sphere" }, { "Steam Centurion", "centurion_steam" },
    { "Scamp", "scamp" }, { "Clannfear", "clannfear" }, { "Dremora", "dremora" }, { "Daedroth", "daedroth" },
    { "Hunger", "hunger" }, { "Ogrim", "ogrim" }, { "Golden Saint", "golden saint" },
    { "Winged Twilight", "winged twilight" }, { "Flame Atronach", "atronach_flame" },
    { "Frost Atronach", "atronach_frost" }, { "Storm Atronach", "atronach_storm" },
    { "Goblin (Tribunal)", "goblin_grunt" }, { "Verminous Fabricant (Tribunal)", "fabricant_verminous" },
    { "Black Bear (Bloodmoon)", "bm_bear_black" }, { "Grey Wolf (Bloodmoon)", "bm_wolf_grey" }
}

playerMenu.times = { { "Dawn (6:00)", 6 }, { "Morning (9:00)", 9 }, { "Noon (12:00)", 12 },
    { "Evening (18:00)", 18 }, { "Night (22:00)", 22 }, { "Midnight (0:00)", 0 } }

-- GUI ids, kept clear of CoreScripts' own (1-4 and 9001-9003)
local ID = { MAIN = 9100, PLAYERS = 9101, PLAYER = 9102, DISGUISE_WHO = 9103, DISGUISE_WHAT = 9104,
    DISGUISE_TYPE = 9105, MESSAGE = 9106, ANIM = 9107, TIME = 9108 }

local state = {}

local function getState(pid)
    if state[pid] == nil then state[pid] = {} end
    return state[pid]
end

local function rank(pid)
    if Players[pid]:IsServerOwner() then return 3
    elseif Players[pid]:IsAdmin() then return 2
    elseif Players[pid]:IsModerator() then return 1 end
    return 0
end

-- Runs a chat command exactly as if the player had typed it
local function run(pid, text)
    eventHandler.OnPlayerSendMessage(pid, "/" .. text)
end

-- Shows a button box built from { { "Caption", function(pid) end, minRank }, ... }
local function showButtons(pid, id, text, entries)
    local s = getState(pid)
    s.buttons = {}
    local captions = {}
    for _, entry in ipairs(entries) do
        if rank(pid) >= (entry[3] or 0) then
            table.insert(s.buttons, entry[2])
            table.insert(captions, entry[1])
        end
    end
    tes3mp.CustomMessageBox(pid, id, text, table.concat(captions, ";"))
end

-- Shows a list; onPick(pid, index) gets a 1-based index into labels
local function showList(pid, id, label, labels, onPick)
    local s = getState(pid)
    s.onPick = onPick
    s.listSize = #labels
    tes3mp.ListBox(pid, id, label, table.concat(labels, "\n"))
end

local function onlinePlayers()
    local pids, labels = {}, {}
    for pid, player in pairs(Players) do
        if player:IsLoggedIn() then
            table.insert(pids, pid)
        end
    end
    table.sort(pids)
    for _, pid in ipairs(pids) do
        table.insert(labels, Players[pid].name .. " (pid " .. pid .. ")")
    end
    return pids, labels
end

local showMain

local function showDisguiseWhat(pid, targetPid)
    local labels = { "Remove disguise", "Type a creature ID..." }
    for _, d in ipairs(playerMenu.disguises) do table.insert(labels, d[1]) end
    showList(pid, ID.DISGUISE_WHAT, "Disguise " .. Players[targetPid].name .. " as:", labels, function(pid, index)
        if index == 1 then
            run(pid, "disguise " .. targetPid)
        elseif index == 2 then
            getState(pid).inputTarget = targetPid
            tes3mp.InputDialog(pid, ID.DISGUISE_TYPE, "Creature ID to disguise " .. Players[targetPid].name .. " as:",
                "Example: golden saint")
        else
            run(pid, "disguise " .. targetPid .. " " .. playerMenu.disguises[index - 2][2])
        end
    end)
end

local function showDisguiseWho(pid)
    local pids, labels = onlinePlayers()
    showList(pid, ID.DISGUISE_WHO, "Who should be disguised?", labels, function(pid, index)
        showDisguiseWhat(pid, pids[index])
    end)
end

local function showPlayer(pid, targetPid)
    local name = Players[targetPid].name
    showButtons(pid, ID.PLAYER, name, {
        { "Send message", function(pid)
            getState(pid).inputTarget = targetPid
            tes3mp.InputDialog(pid, ID.MESSAGE, "Message to " .. name .. ":", "")
        end },
        { "Invite as ally", function(pid) run(pid, "invite " .. targetPid) end },
        { "Accept their invite", function(pid) run(pid, "join " .. targetPid) end },
        { "Leave alliance", function(pid) run(pid, "leave " .. targetPid) end },
        { "Go to them", function(pid) run(pid, "teleportto " .. targetPid) end, 1 },
        { "Bring them here", function(pid) run(pid, "teleport " .. targetPid) end, 1 },
        { "Disguise", function(pid) showDisguiseWhat(pid, targetPid) end, 2 },
        { "Kick", function(pid) run(pid, "kick " .. targetPid) end, 1 },
        { "Back", function(pid) showMain(pid) end }
    })
end

local function showPlayers(pid)
    local pids, labels = onlinePlayers()
    showList(pid, ID.PLAYERS, "Players online", labels, function(pid, index)
        showPlayer(pid, pids[index])
    end)
end

local function showAnims(pid)
    local labels = {}
    for anim in animHelper.GetValidList(pid):gmatch("[^,%s]+") do table.insert(labels, anim) end
    table.sort(labels)
    showList(pid, ID.ANIM, "Play an animation", labels, function(pid, index)
        run(pid, "anim " .. labels[index])
    end)
end

local function showTime(pid)
    local labels = {}
    for _, t in ipairs(playerMenu.times) do table.insert(labels, t[1]) end
    showList(pid, ID.TIME, "Set the time of day", labels, function(pid, index)
        run(pid, "sethour " .. playerMenu.times[index][2])
    end)
end

showMain = function(pid)
    showButtons(pid, ID.MAIN, color.Orange .. "Menu", {
        { "Players", showPlayers },
        { "Animations", showAnims },
        { "Disguise", showDisguiseWho, 2 },
        { "Time of day", showTime, 1 },
        { "Crafting", function(pid) run(pid, "craft") end },
        { "Unstuck", function(pid) run(pid, "fixme") end },
        { "All commands", function(pid) run(pid, "help") end },
        { "Close", nil }
    })
end

customCommandHooks.registerCommand("menu", function(pid, cmd) showMain(pid) end)

customEventHooks.registerHandler("OnGUIAction", function(eventStatus, pid, idGui, data)
    local s = state[pid]
    if s == nil or Players[pid] == nil or not Players[pid]:IsLoggedIn() then return end

    if idGui == ID.MAIN or idGui == ID.PLAYER then
        local action = s.buttons and s.buttons[(tonumber(data) or -1) + 1]
        s.buttons = nil
        if action then action(pid) end

    elseif idGui == ID.PLAYERS or idGui == ID.DISGUISE_WHO or idGui == ID.DISGUISE_WHAT
        or idGui == ID.ANIM or idGui == ID.TIME then
        -- Lists send the 0-based row, or an out-of-range number when closed without a pick
        local index = (tonumber(data) or -1) + 1
        local onPick = s.onPick
        s.onPick = nil
        if onPick and index >= 1 and index <= (s.listSize or 0) then onPick(pid, index) end

    elseif idGui == ID.DISGUISE_TYPE then
        local refId = tostring(data):match("^%s*(.-)%s*$")
        if refId ~= "" and s.inputTarget ~= nil and Players[s.inputTarget] ~= nil then
            run(pid, "disguise " .. s.inputTarget .. " " .. refId)
        end

    elseif idGui == ID.MESSAGE then
        local text = tostring(data)
        if text:match("%S") and s.inputTarget ~= nil and Players[s.inputTarget] ~= nil then
            run(pid, "msg " .. s.inputTarget .. " " .. text)
        end
    end
end)

customEventHooks.registerHandler("OnPlayerDisconnect", function(eventStatus, pid)
    state[pid] = nil
end)

return playerMenu
