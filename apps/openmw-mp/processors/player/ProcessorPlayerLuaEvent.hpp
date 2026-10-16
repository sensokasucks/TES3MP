#ifndef OPENMW_PROCESSORPLAYERLUAEVENT_HPP
#define OPENMW_PROCESSORPLAYERLUAEVENT_HPP

#include "../PlayerProcessor.hpp"
#include "Script/Functions/Hooks.hpp"

namespace mwmp
{
    class ProcessorPlayerLuaEvent : public PlayerProcessor
    {
    public:
        ProcessorPlayerLuaEvent()
        {
            BPP_INIT(ID_PLAYER_LUA_EVENT)
        }

        void Do(PlayerPacket &packet, Player &player) override
        {
            DEBUG_PRINTF(strPacketID.c_str());

            if (!Hooks::allowLuaEvent(player))
                return;

            Script::Call<Script::CallbackIdentity("OnPlayerLuaEvent")>(player.getId(), player.luaEventName.c_str(),
                player.luaEventPayload.c_str());
        }
    };
}

#endif //OPENMW_PROCESSORPLAYERLUAEVENT_HPP
