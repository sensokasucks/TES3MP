-- sharedStorage.lua: storage sections shared by every player, kept by the server and saved to
-- data/custom/sharedStorage.json. Client Lua mods use them through I.Multiplayer.sharedSection(name).
--
-- Needs custom/luaEvents.lua. Install: copy to server/scripts/custom/ and add
-- require("custom.sharedStorage") to customScripts.lua after luaEvents.
--
-- Server scripts:
--   local sharedStorage = require("custom.sharedStorage")
--   sharedStorage.get(section, key)
--   sharedStorage.set(section, key, value)            value nil deletes; sent to every subscribed player
--   sharedStorage.protect(section, minStaffRank)      players below the rank can't write (0 = anyone)
--   sharedStorage.registerValidator(section, function(pid, key, value) return true end)
--                                                     return false to reject a player's write

local luaEvents = require("custom.luaEvents")

local sharedStorage = {}

local fileName = "custom/sharedStorage.json"
local maxKeyLength = 100
local maxKeysPerSection = 1000
local saveIntervalMs = 30000

-- JSON libraries guess whether a table is an array from its keys (dkjson saves { n = 5 } as an
-- array), so sections are saved and sent as lists of { key = ..., value = ... } pairs
local function toPairs(values)
    local list = {}
    for key, value in pairs(values or {}) do
        table.insert(list, { key = key, value = value })
    end
    return list
end

local sections = {}
for name, list in pairs(jsonInterface.load(fileName) or {}) do
    sections[name] = {}
    for _, entry in ipairs(type(list) == "table" and list or {}) do
        if type(entry) == "table" and type(entry.key) == "string" then
            sections[name][entry.key] = entry.value
        end
    end
end
local subscribers = {} -- section -> { [pid] = true }
local protections = {} -- section -> minimum staff rank
local validators = {} -- section -> { function(pid, key, value) }
local dirty = false

local function countKeys(t)
    local n = 0
    for _ in pairs(t) do n = n + 1 end
    return n
end

local function broadcast(section, key, value)
    local list = subscribers[section]
    if list == nil then return end
    for pid in pairs(list) do
        luaEvents.send(pid, "storage.update", { section = section, key = key, value = value })
    end
end

function sharedStorage.get(section, key)
    return sections[section] and sections[section][key]
end

function sharedStorage.set(section, key, value)
    sections[section] = sections[section] or {}
    sections[section][key] = value
    dirty = true
    broadcast(section, key, value)
end

function sharedStorage.protect(section, minStaffRank)
    protections[section] = minStaffRank
end

function sharedStorage.registerValidator(section, validator)
    validators[section] = validators[section] or {}
    table.insert(validators[section], validator)
end

function sharedStorage.save()
    if dirty then
        local saved = {}
        for name, values in pairs(sections) do
            saved[name] = toPairs(values)
        end
        jsonInterface.quicksave(fileName, saved)
        dirty = false
    end
end

local function validSectionName(section)
    return type(section) == "string" and section ~= "" and #section <= maxKeyLength
end

luaEvents.register("storage.subscribe", function(pid, data)
    if type(data) ~= "table" or not validSectionName(data.section) then return end

    subscribers[data.section] = subscribers[data.section] or {}
    subscribers[data.section][pid] = true

    luaEvents.send(pid, "storage.sync", { section = data.section, entries = toPairs(sections[data.section]) })
end)

luaEvents.register("storage.set", function(pid, data)
    if type(data) ~= "table" or not validSectionName(data.section) then return end

    local section, key, value = data.section, data.key, data.value

    if type(key) ~= "string" or key == "" or #key > maxKeyLength then return end

    local rank = protections[section]
    if rank ~= nil and rank > 0 and Players[pid].data.settings.staffRank < rank then
        tes3mp.LogMessage(enumerations.log.WARN, "[sharedStorage] " .. logicHandler.GetChatName(pid) ..
            " may not write to protected section " .. section)
        luaEvents.send(pid, "storage.update", { section = section, key = key, value = sharedStorage.get(section, key) })
        return
    end

    if value ~= nil and sections[section] ~= nil and sections[section][key] == nil
        and countKeys(sections[section]) >= maxKeysPerSection then
        tes3mp.LogMessage(enumerations.log.WARN, "[sharedStorage] section " .. section .. " is full")
        return
    end

    for _, validator in ipairs(validators[section] or {}) do
        if not validator(pid, key, value) then
            -- Tell the writer the value it should have instead
            luaEvents.send(pid, "storage.update", { section = section, key = key, value = sharedStorage.get(section, key) })
            return
        end
    end

    sharedStorage.set(section, key, value)
end)

customEventHooks.registerHandler("OnPlayerDisconnect", function(eventStatus, pid)
    for _, list in pairs(subscribers) do
        list[pid] = nil
    end
end)

customEventHooks.registerHandler("OnServerExit", function(eventStatus)
    sharedStorage.save()
end)

function SharedStorageSaveTimer()
    sharedStorage.save()
    tes3mp.RestartTimer(sharedStorage.saveTimer, saveIntervalMs)
end

sharedStorage.saveTimer = tes3mp.CreateTimer("SharedStorageSaveTimer", saveIntervalMs)
tes3mp.StartTimer(sharedStorage.saveTimer)

return sharedStorage
