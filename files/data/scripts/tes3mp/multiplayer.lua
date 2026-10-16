-- Built-in TES3MP player script: connects OpenMW Lua mods to server scripts.
--
-- Receiving: an event the server sends with tes3mp.SendLuaEvent(pid, "name", payload) arrives on the
-- player as the local event "MP_name", with the JSON payload decoded:
--     eventHandlers = { MP_myEvent = function(data) ... end }
-- Sending: I.Multiplayer.send("name", data) encodes data as JSON and reaches the server's
-- OnPlayerLuaEvent(pid, "name", payload). It returns false when the event is too large
-- (16 KB) or the script sends more than 30 events per second.

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

-- Events this script handles itself, besides forwarding them
local builtinHandlers = {
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
        encodeJson = json.encode,
        decodeJson = json.decode,
    },
    engineHandlers = {
        onFrame = onFrame,
    },
}
