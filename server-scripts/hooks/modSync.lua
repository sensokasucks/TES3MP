-- modSync.lua: lets each mod's multiplayer sync settings live in one data file instead of edits
-- to config.lua and clientVariableScopes.lua.
--
-- Install: copy to server/scripts/custom/ and add require("custom.modSync") to customScripts.lua.
-- Then create data/custom/modSync.json, with one entry per plugin (mod authors can ship theirs):
--
-- {
--     "MyMod.esp": {
--         "synchronizedScripts": ["mymod_doorscript"],
--         "globals": {
--             "worldwide": ["mymod_bridgeraised"],
--             "quest": ["mymod_queststage"],
--             "personal": ["mymod_playerflag"],
--             "kills": [], "factionRanks": [], "factionExpulsion": [],
--             "ignored": ["mymod_timer"]
--         }
--     }
-- }
--
-- An entry only applies when its plugin is in requiredDataFiles.json; use "*" for always.
-- Categories mean the same as in clientVariableScopes.lua.

local fileName = "custom/modSync.json"
local categories = { "ignored", "personal", "quest", "kills", "factionRanks", "factionExpulsion", "worldwide" }

local function isLoaded(pluginName)
    return pluginName == "*" or tableHelper.containsCaseInsensitiveString(clientDataFiles, pluginName)
end

local function addUnique(list, value)
    if not tableHelper.containsCaseInsensitiveString(list, value) then
        table.insert(list, value)
    end
end

customEventHooks.registerHandler("OnServerPostInit", function(eventStatus)
    local entries = jsonInterface.load(fileName)
    if entries == nil then
        tes3mp.LogMessage(enumerations.log.INFO, "[modSync] no " .. fileName .. ", nothing to add")
        return
    end

    for pluginName, entry in pairs(entries) do
        if type(entry) ~= "table" then
            tes3mp.LogMessage(enumerations.log.WARN, "[modSync] entry for " .. tostring(pluginName) .. " is not an object")
        elseif not isLoaded(pluginName) then
            tes3mp.LogMessage(enumerations.log.INFO, "[modSync] skipping " .. pluginName .. ": not in requiredDataFiles.json")
        else
            local scriptCount, globalCount = 0, 0

            for _, scriptId in ipairs(entry.synchronizedScripts or {}) do
                addUnique(config.synchronizedClientScriptIds, scriptId)
                scriptCount = scriptCount + 1
            end

            for _, category in ipairs(categories) do
                local ids = entry.globals and entry.globals[category]
                if ids ~= nil then
                    clientVariableScopes.globals[category] = clientVariableScopes.globals[category] or {}
                    for _, globalId in ipairs(ids) do
                        addUnique(clientVariableScopes.globals[category], globalId)
                        globalCount = globalCount + 1
                    end
                end
            end

            tes3mp.LogMessage(enumerations.log.INFO, string.format("[modSync] %s: %d scripts, %d globals",
                pluginName, scriptCount, globalCount))
        end
    end
end)
