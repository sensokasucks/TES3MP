-- luaEvents.lua: JSON event channel between server scripts and client OpenMW Lua mods.
--
-- Install: copy to server/scripts/custom/ and add require("custom.luaEvents") to customScripts.lua
-- (before scripts that use it). Other scripts get it with: local luaEvents = require("custom.luaEvents")
--
--   luaEvents.send(pid, name, data)        send a table (or string/number/bool) to one player
--   luaEvents.sendToAll(name, data)        send to every logged-in player
--   luaEvents.toast(pid, text)             show a message box on the player's screen
--   luaEvents.register(name, function(pid, data) ... end)
--                                          handle events client mods send with I.Multiplayer.send(name, data)
--
-- Only names that were registered are accepted from clients; anything else is logged and dropped.
-- Client data is untrusted: check it before acting on it.

local dkjson = require("dkjson")

local luaEvents = {}
local handlers = {}

local function isLoggedIn(pid)
    return Players[pid] ~= nil and Players[pid]:IsLoggedIn()
end

local function encode(data)
    if data == nil then return "" end
    return dkjson.encode(data)
end

function luaEvents.send(pid, name, data)
    if not isLoggedIn(pid) then return false end
    return tes3mp.SendLuaEvent(pid, name, encode(data))
end

function luaEvents.sendToAll(name, data)
    local payload = encode(data)
    for pid, player in pairs(Players) do
        if player:IsLoggedIn() then
            tes3mp.SendLuaEvent(pid, name, payload)
        end
    end
end

function luaEvents.toast(pid, text)
    return luaEvents.send(pid, "toast", { text = text })
end

function luaEvents.register(name, handler)
    handlers[name] = handlers[name] or {}
    table.insert(handlers[name], handler)
end

function OnPlayerLuaEvent(pid, name, payload)
    if not isLoggedIn(pid) then return end

    local list = handlers[name]
    if list == nil then
        tes3mp.LogMessage(enumerations.log.WARN, "[luaEvents] " .. logicHandler.GetChatName(pid) ..
            " sent unregistered event " .. tostring(name))
        return
    end

    local data = nil
    if payload ~= "" then
        local decoded, _, err = dkjson.decode(payload)
        if err ~= nil then
            tes3mp.LogMessage(enumerations.log.WARN, "[luaEvents] bad JSON in " .. name .. " from " ..
                logicHandler.GetChatName(pid) .. ": " .. tostring(err))
            return
        end
        data = decoded
    end

    for _, handler in ipairs(list) do
        local ok, err = pcall(handler, pid, data)
        if not ok then
            tes3mp.LogMessage(enumerations.log.ERROR, "[luaEvents] handler for " .. name .. " failed: " .. tostring(err))
        end
    end
end

-- Built-in round trip check: /luaping sends "ping", the client's Multiplayer script answers "pong"
local pingTimes = {}

luaEvents.register("pong", function(pid, data)
    local sentAt = pingTimes[pid]
    if sentAt == nil then return end
    pingTimes[pid] = nil
    tes3mp.SendMessage(pid, string.format("Client Lua answered in %d ms.\n", tes3mp.GetMillisecondsSinceServerStart() - sentAt), false)
end)

customCommandHooks.registerCommand("luaping", function(pid, cmd)
    pingTimes[pid] = tes3mp.GetMillisecondsSinceServerStart()
    luaEvents.send(pid, "ping", { sent = os.time() })
    luaEvents.toast(pid, "Hello from the server's Lua!")
end)

customEventHooks.registerHandler("OnPlayerDisconnect", function(eventStatus, pid)
    pingTimes[pid] = nil
end)

return luaEvents
