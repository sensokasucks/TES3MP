#ifndef OPENMW_PROCESSORPLAYERATTACK_HPP
#define OPENMW_PROCESSORPLAYERATTACK_HPP

#include "../PlayerProcessor.hpp"
#include "Script/Functions/Hooks.hpp"

namespace mwmp
{
    class ProcessorPlayerAttack : public PlayerProcessor
    {
        PlayerPacketController *playerController;
    public:
        ProcessorPlayerAttack()
        {
            BPP_INIT(ID_PLAYER_ATTACK)
        }

        void Do(PlayerPacket &packet, Player &player) override
        {
            DEBUG_PRINTF(strPacketID.c_str());

            if (!player.creatureStats.mDead)
            {
                player.sendToLoaded(&packet);
            }

            Hooks::markActive(player);

            if (player.attack.isHit)
                Script::Call<Script::CallbackIdentity("OnPlayerAttack")>(player.getId());
        }
    };
}

#endif //OPENMW_PROCESSORPLAYERATTACK_HPP
