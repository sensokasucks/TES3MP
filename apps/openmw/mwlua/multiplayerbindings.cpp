#include "multiplayerbindings.hpp"

#include <components/lua/luastate.hpp>

#include "context.hpp"

#include "../mwmp/LocalPlayer.hpp"
#include "../mwmp/LuaChannel.hpp"
#include "../mwmp/Main.hpp"

namespace MWLua
{
    sol::table initMultiplayerPackage(const Context& context)
    {
        sol::state_view lua = context.sol();
        sol::table api(lua, sol::create);

        api["isMultiplayer"] = []() { return mwmp::Main::isInitialized(); };

        // False until the server has accepted this player's character (login or character creation finished)
        api["isLoggedIn"] = []() {
            return mwmp::Main::isInitialized() && mwmp::Main::get().getLocalPlayer()->isLoggedIn();
        };

        api["sendToServer"] = [](std::string_view name, std::string_view payload) {
            return mwmp::LuaChannel::pushOutgoing({ std::string(name), std::string(payload) });
        };

        // Returns the events the server sent since the last call, oldest first, as { name = ..., payload = ... }
        api["receive"] = [lua]() {
            sol::table result(lua, sol::create);
            int index = 1;
            for (auto& event : mwmp::LuaChannel::takeIncoming())
            {
                sol::table entry(lua, sol::create);
                entry["name"] = std::move(event.name);
                entry["payload"] = std::move(event.payload);
                result[index++] = entry;
            }
            return result;
        };

        return LuaUtil::makeReadOnly(api);
    }
}
