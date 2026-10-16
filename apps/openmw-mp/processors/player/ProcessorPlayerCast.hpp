#ifndef OPENMW_PROCESSORPLAYERCAST_HPP
#define OPENMW_PROCESSORPLAYERCAST_HPP

#include "../PlayerProcessor.hpp"
#include "Script/Functions/Hooks.hpp"

namespace mwmp
{
    class ProcessorPlayerCast : public PlayerProcessor
    {
        PlayerPacketController *playerController;
    public:
        ProcessorPlayerCast()
        {
            BPP_INIT(ID_PLAYER_CAST)
        }

        void Do(PlayerPacket &packet, Player &player) override
        {
            DEBUG_PRINTF(strPacketID.c_str());

            if (!player.creatureStats.mDead)
            {
                player.sendToLoaded(&packet);
            }

            Hooks::markActive(player);

            // Spell casts are sent once when pressed and again with their result
            if (player.cast.type == mwmp::Cast::ITEM || !player.cast.pressed)
                Script::Call<Script::CallbackIdentity("OnPlayerCast")>(player.getId());
        }
    };
}

#endif //OPENMW_PROCESSORPLAYERCAST_HPP
