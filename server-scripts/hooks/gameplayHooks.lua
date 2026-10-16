-- gameplayHooks.lua: routes the port's new gameplay events into customEventHooks.
--
-- Install: copy to server/scripts/custom/ and add require("custom.gameplayHooks")
-- to server/scripts/customScripts.lua, before any script that uses these events.
--
-- Events (register with customEventHooks.registerHandler):
--   OnPlayerAttack(eventStatus, pid)                       an attack by pid hit or missed a target;
--                                                           read details with tes3mp.GetAttack*(pid)
--   OnPlayerCast(eventStatus, pid)                         pid finished casting a spell or used an enchanted item;
--                                                           read details with tes3mp.GetCast*(pid)
--   OnPlayerHealthThreshold(eventStatus, pid, threshold, falling)
--                                                           pid's health crossed a fraction added with
--                                                           tes3mp.AddHealthThreshold (falling = went below it)
--   OnPlayerMenu(eventStatus, pid, menu)                   pid opened, changed or closed a menu
--                                                           ("" = closed; e.g. "inventory", "dialogue", "chargen")
-- Getters: tes3mp.GetPlayerMenu(pid), tes3mp.IsPlayerInMenu(pid), tes3mp.GetPlayerIdleTime(pid) (seconds).

local function isLoggedIn(pid)
    return Players[pid] ~= nil and Players[pid]:IsLoggedIn()
end

local function trigger(event, pid, ...)
    if not isLoggedIn(pid) then return end
    local eventStatus = customEventHooks.triggerValidators(event, {pid, ...})
    customEventHooks.triggerHandlers(event, eventStatus, {pid, ...})
end

function OnPlayerAttack(pid)
    trigger("OnPlayerAttack", pid)
end

function OnPlayerCast(pid)
    trigger("OnPlayerCast", pid)
end

function OnPlayerHealthThreshold(pid, threshold, falling)
    trigger("OnPlayerHealthThreshold", pid, threshold, falling)
end

function OnPlayerMenu(pid, menu)
    trigger("OnPlayerMenu", pid, menu)
end
