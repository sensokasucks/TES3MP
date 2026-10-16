-- hooksDemo.lua: logs every new gameplay event to the server log and adds /idle.
-- For testing the hooks; remove it once you've seen them fire.
--
-- Needs custom/gameplayHooks.lua. Install: copy to server/scripts/custom/ and add
-- require("custom.hooksDemo") to customScripts.lua after gameplayHooks.

local function name(pid)
    return logicHandler.GetChatName(pid)
end

local function describeTarget(isPlayer, targetPid, refId)
    if isPlayer then
        return targetPid >= 0 and name(targetPid) or "a player"
    end
    return refId ~= "" and refId or "nothing"
end

tes3mp.AddHealthThreshold(0.5)
tes3mp.AddHealthThreshold(0.25)

customEventHooks.registerHandler("OnPlayerAttack", function(eventStatus, pid)
    local target = describeTarget(tes3mp.GetAttackTargetIsPlayer(pid), tes3mp.GetAttackTargetPid(pid),
        tes3mp.GetAttackTargetRefId(pid))
    tes3mp.LogMessage(enumerations.log.INFO, string.format("[hooks] %s attacked %s: %s, damage %.1f%s",
        name(pid), target, tes3mp.GetAttackSuccess(pid) and "hit" or "miss", tes3mp.GetAttackDamage(pid),
        tes3mp.GetAttackBlocked(pid) and " (blocked)" or ""))
end)

customEventHooks.registerHandler("OnPlayerCast", function(eventStatus, pid)
    local what = tes3mp.GetCastType(pid) == 1 and tes3mp.GetCastItemId(pid) or tes3mp.GetCastSpellId(pid)
    local target = describeTarget(tes3mp.GetCastTargetIsPlayer(pid), tes3mp.GetCastTargetPid(pid),
        tes3mp.GetCastTargetRefId(pid))
    tes3mp.LogMessage(enumerations.log.INFO, string.format("[hooks] %s cast %s at %s: %s",
        name(pid), what, target, tes3mp.GetCastSuccess(pid) and "success" or "failed"))
end)

customEventHooks.registerHandler("OnPlayerHealthThreshold", function(eventStatus, pid, threshold, falling)
    tes3mp.LogMessage(enumerations.log.INFO, string.format("[hooks] %s health went %s %d%%",
        name(pid), falling and "below" or "back above", math.floor(threshold * 100 + 0.5)))
end)

customEventHooks.registerHandler("OnPlayerMenu", function(eventStatus, pid, menu)
    tes3mp.LogMessage(enumerations.log.INFO, string.format("[hooks] %s menu: %s",
        name(pid), menu == "" and "(closed)" or menu))
end)

customCommandHooks.registerCommand("idle", function(pid, cmd)
    local lines = {}
    for otherPid, player in pairs(Players) do
        if player:IsLoggedIn() then
            local menu = tes3mp.GetPlayerMenu(otherPid)
            table.insert(lines, string.format("%s: idle %ds%s", name(otherPid),
                math.floor(tes3mp.GetPlayerIdleTime(otherPid)), menu ~= "" and (", in " .. menu) or ""))
        end
    end
    tes3mp.SendMessage(pid, table.concat(lines, "\n") .. "\n", false)
end)
