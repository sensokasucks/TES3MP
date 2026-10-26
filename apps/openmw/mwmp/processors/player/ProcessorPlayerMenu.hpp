#ifndef OPENMW_PROCESSORPLAYERMENU_HPP
#define OPENMW_PROCESSORPLAYERMENU_HPP

#include "../PlayerProcessor.hpp"

namespace mwmp
{
    // Another player opened or closed a menu. The packet has already stored the menu name and the
    // dialogue partner on the DedicatedPlayer; MechanicsHelper::getDialoguePartner reads them from there.
    class ProcessorPlayerMenu final: public PlayerProcessor
    {
    public:
        ProcessorPlayerMenu()
        {
            BPP_INIT(ID_PLAYER_MENU)
        }

        virtual void Do(PlayerPacket &packet, BasePlayer *player)
        {
        }
    };
}

#endif //OPENMW_PROCESSORPLAYERMENU_HPP
