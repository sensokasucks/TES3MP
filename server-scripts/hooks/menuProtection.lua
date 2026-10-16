-- menuProtection.lua: players can't be hurt by other players while they are in
-- dialogue, character creation, loading or jail. Hits that land anyway are undone:
-- health and fatigue go back to what they were when the menu opened.
--
-- Needs custom/gameplayHooks.lua. Install: copy to server/scripts/custom/ and add
-- require("custom.menuProtection") to customScripts.lua after gameplayHooks.

local protectedMenus = {
    dialogue = true, chargen = true, loading = true, jail = true
}

-- pid -> { health, fatigue } recorded when a protected menu opened
local snapshots = {}
-- pid -> reusable restore timer
local timers = {}

local function isLoggedIn(pid)
    return Players[pid] ~= nil and Players[pid]:IsLoggedIn()
end

customEventHooks.registerHandler("OnPlayerMenu", function(eventStatus, pid, menu)
    if protectedMenus[menu] then
        if snapshots[pid] == nil then
            snapshots[pid] = { health = tes3mp.GetHealthCurrent(pid), fatigue = tes3mp.GetFatigueCurrent(pid) }
        end
    else
        snapshots[pid] = nil
    end
end)

customEventHooks.registerHandler("OnPlayerDisconnect", function(eventStatus, pid)
    snapshots[pid] = nil
    if timers[pid] ~= nil then
        tes3mp.StopTimer(timers[pid])
        tes3mp.FreeTimer(timers[pid])
        timers[pid] = nil
    end
end)

-- The victim's client applies the hit after the attacker's packet reaches the server,
-- so restore a moment later
function MenuProtectionRestore(pid)
    local snapshot = snapshots[pid]
    if snapshot == nil or not isLoggedIn(pid) then return end

    local changed = false
    if tes3mp.GetHealthCurrent(pid) < snapshot.health then
        tes3mp.SetHealthCurrent(pid, snapshot.health)
        changed = true
    end
    if tes3mp.GetFatigueCurrent(pid) < snapshot.fatigue then
        tes3mp.SetFatigueCurrent(pid, snapshot.fatigue)
        changed = true
    end
    if changed then
        tes3mp.SendStatsDynamic(pid)
    end
end

customEventHooks.registerHandler("OnPlayerAttack", function(eventStatus, pid)
    if not tes3mp.GetAttackTargetIsPlayer(pid) or not tes3mp.GetAttackSuccess(pid) then return end

    local targetPid = tes3mp.GetAttackTargetPid(pid)
    if targetPid < 0 or snapshots[targetPid] == nil or not isLoggedIn(targetPid) then return end

    if timers[targetPid] == nil then
        timers[targetPid] = tes3mp.CreateTimerEx("MenuProtectionRestore", 500, "i", targetPid)
        tes3mp.StartTimer(timers[targetPid])
    else
        tes3mp.RestartTimer(timers[targetPid], 500)
    end

    tes3mp.SendMessage(pid, color.Warning .. logicHandler.GetChatName(targetPid) ..
        " is busy and can't be hurt right now.\n", false)
end)
