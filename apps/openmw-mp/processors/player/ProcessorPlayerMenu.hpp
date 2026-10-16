#ifndef OPENMW_PROCESSORPLAYERMENU_HPP
#define OPENMW_PROCESSORPLAYERMENU_HPP

#include "../PlayerProcessor.hpp"
#include "Script/Functions/Hooks.hpp"

namespace mwmp
{
    class ProcessorPlayerMenu : public PlayerProcessor
    {
    public:
        ProcessorPlayerMenu()
        {
            BPP_INIT(ID_PLAYER_MENU)
        }

        void Do(PlayerPacket &packet, Player &player) override
        {
            DEBUG_PRINTF(strPacketID.c_str());

            Hooks::markActive(player);

            Script::Call<Script::CallbackIdentity("OnPlayerMenu")>(player.getId(), player.menu.c_str());
        }
    };
}

#endif //OPENMW_PROCESSORPLAYERMENU_HPP
