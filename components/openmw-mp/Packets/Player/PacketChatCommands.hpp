#ifndef OPENMW_PACKETCHATCOMMANDS_HPP
#define OPENMW_PACKETCHATCOMMANDS_HPP

#include <components/openmw-mp/Packets/Player/PlayerPacket.hpp>

namespace mwmp
{
    class PacketChatCommands : public PlayerPacket
    {
    public:
        PacketChatCommands(RakNet::RakPeerInterface *peer);

        virtual void Packet(RakNet::BitStream *newBitstream, bool send);
    };
}

#endif //OPENMW_PACKETCHATCOMMANDS_HPP
