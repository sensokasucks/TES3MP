#ifndef OPENMW_PROCESSORPLAYERLUAEVENT_HPP
#define OPENMW_PROCESSORPLAYERLUAEVENT_HPP

#include "../PlayerProcessor.hpp"
#include "apps/openmw/mwmp/LuaChannel.hpp"

namespace mwmp
{
    class ProcessorPlayerLuaEvent final: public PlayerProcessor
    {
    public:
        ProcessorPlayerLuaEvent()
        {
            BPP_INIT(ID_PLAYER_LUA_EVENT)
        }

        virtual void Do(PlayerPacket &packet, BasePlayer *player)
        {
            if (!isLocal()) return;

            LuaChannel::pushIncoming({ player->luaEventName, player->luaEventPayload });
        }
    };
}

#endif //OPENMW_PROCESSORPLAYERLUAEVENT_HPP
