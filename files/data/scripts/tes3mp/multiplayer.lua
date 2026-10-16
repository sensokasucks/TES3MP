-- Built-in TES3MP player script: connects OpenMW Lua mods to server scripts.
--
-- Receiving: an event the server sends with tes3mp.SendLuaEvent(pid, "name", payload) arrives on the
-- player as the local event "MP_name", with the JSON payload decoded:
--     eventHandlers = { MP_myEvent = function(data) ... end }
-- Sending: I.Multiplayer.send("name", data) encodes data as JSON and reaches the server's
-- OnPlayerLuaEvent(pid, "name", payload). It returns false when the event is too large
-- (16 KB) or the script sends more than 30 events per second.
--
-- Shared storage (needs the server's sharedStorage.lua): I.Multiplayer.sharedSection(name) returns
-- a section every player sees the same way, kept and saved by the server:
--     local section = I.Multiplayer.sharedSection('MyMod')
--     section:get(key)              nil until the server's copy has arrived (see isReady)
--     section:set(key, value)       value is any JSON-able value, nil deletes; the change shows up in
--                                   get() once the server has accepted it
--     section:asTable()             copy of every key and value
--     section:isReady()
--     section:subscribe(function(key) end)  called after each change (key is nil after a full sync)

local mp = require('openmw.multiplayer')
local self = require('openmw.self')
local ui = require('openmw.ui')
local json = require('scripts.tes3mp.json')

local function send(name, data)
    if type(name) ~= 'string' or name == '' then
        error('Multiplayer.send: name must be a non-empty string', 2)
    end
    local payload = data == nil and '' or json.encode(data)
    return mp.sendToServer(name, payload)
end

local sections = {}

local function deepCopy(value)
    if type(value) ~= 'table' then return value end
    local copy = {}
    for k, v in pairs(value) do copy[k] = deepCopy(v) end
    return copy
end

local function notify(section, key)
    for _, callback in ipairs(section.callbacks) do
        local ok, err = pcall(callback, key)
        if not ok then print('Multiplayer: shared section ' .. section.name .. ' callback failed: ' .. tostring(err)) end
    end
end

local Section = {}
Section.__index = Section

function Section:get(key) return deepCopy(self.values[key]) end
function Section:asTable() return deepCopy(self.values) end
function Section:isReady() return self.ready end
function Section:subscribe(callback) table.insert(self.callbacks, callback) end
function Section:set(key, value)
    if type(key) ~= 'string' or key == '' then error('sharedSection:set: key must be a non-empty string', 2) end
    return send('storage.set', { section = self.name, key = key, value = value })
end

local function sharedSection(name)
    if type(name) ~= 'string' or name == '' then error('sharedSection: name must be a non-empty string', 2) end
    if sections[name] == nil then
        sections[name] = setmetatable({ name = name, values = {}, ready = false, callbacks = {} }, Section)
        send('storage.subscribe', { section = name })
    end
    return sections[name]
end

-- Events this script handles itself, besides forwarding them
local builtinHandlers = {
    ['storage.sync'] = function(data)
        local section = type(data) == 'table' and sections[data.section]
        if not section then return end
        section.values = {}
        for _, entry in ipairs(type(data.entries) == 'table' and data.entries or {}) do
            if type(entry) == 'table' and type(entry.key) == 'string' then
                section.values[entry.key] = entry.value
            end
        end
        section.ready = true
        notify(section, nil)
    end,
    ['storage.update'] = function(data)
        local section = type(data) == 'table' and sections[data.section]
        if not section or type(data.key) ~= 'string' then return end
        section.values[data.key] = data.value
        notify(section, data.key)
    end,
    toast = function(data)
        if type(data) == 'table' and data.text ~= nil then
            ui.showMessage(tostring(data.text))
        elseif type(data) == 'string' and data ~= '' then
            ui.showMessage(data)
        end
    end,
    ping = function(data)
        send('pong', data)
    end,
}

local function onFrame()
    for _, event in ipairs(mp.receive()) do
        local data = event.payload
        if data ~= '' then
            local ok, decoded = pcall(json.decode, data)
            if ok then data = decoded end
        end

        local handler = builtinHandlers[event.name]
        if handler then
            local ok, err = pcall(handler, data)
            if not ok then print('Multiplayer: handler for ' .. event.name .. ' failed: ' .. tostring(err)) end
        end

        self:sendEvent('MP_' .. event.name, data)
    end
end

return {
    interfaceName = 'Multiplayer',
    interface = {
        version = 1,
        isMultiplayer = mp.isMultiplayer,
        send = send,
        sharedSection = sharedSection,
        encodeJson = json.encode,
        decodeJson = json.decode,
    },
    engineHandlers = {
        onFrame = onFrame,
    },
}
