#include <components/openmw-mp/NetworkMessages.hpp>
#include "PacketChatCommands.hpp"

using namespace mwmp;

static const uint32_t maxCommands = 1024;

PacketChatCommands::PacketChatCommands(RakNet::RakPeerInterface *peer) : PlayerPacket(peer)
{
    packetID = ID_CHAT_COMMANDS;
    orderChannel = CHANNEL_SYSTEM;
}

void PacketChatCommands::Packet(RakNet::BitStream *newBitstream, bool send)
{
    PlayerPacket::Packet(newBitstream, send);

    uint32_t count;

    if (send)
        count = static_cast<uint32_t>(player->chatCommands.size());

    RW(count, send);

    if (!send)
    {
        if (count > maxCommands)
        {
            packetValid = false;
            return;
        }

        player->chatCommands.clear();
        player->chatCommands.resize(count);
    }

    for (auto &&command : player->chatCommands)
    {
        RW(command.name, send, true);
        RW(command.usage, send, true);
        RW(command.description, send, true);
    }
}
