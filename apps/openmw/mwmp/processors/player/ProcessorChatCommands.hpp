#ifndef OPENMW_PROCESSORCHATCOMMANDS_HPP
#define OPENMW_PROCESSORCHATCOMMANDS_HPP


#include "../PlayerProcessor.hpp"
#include "apps/openmw/mwmp/Main.hpp"
#include "apps/openmw/mwmp/GUIController.hpp"

namespace mwmp
{
    class ProcessorChatCommands final: public PlayerProcessor
    {
    public:
        ProcessorChatCommands()
        {
            BPP_INIT(ID_CHAT_COMMANDS)
        }

        virtual void Do(PlayerPacket &packet, BasePlayer *player)
        {
            if (!isLocal()) return;

            LOG_MESSAGE_SIMPLE(TimedLog::LOG_INFO, "Received ID_CHAT_COMMANDS with %zu commands", player->chatCommands.size());

            Main::get().getGUIController()->setChatCommands(player->chatCommands);
        }
    };
}

#endif //OPENMW_PROCESSORCHATCOMMANDS_HPP
