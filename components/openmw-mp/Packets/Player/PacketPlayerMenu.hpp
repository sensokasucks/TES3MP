#ifndef OPENMW_PACKETPLAYERMENU_HPP
#define OPENMW_PACKETPLAYERMENU_HPP

#include <components/openmw-mp/Packets/Player/PlayerPacket.hpp>

namespace mwmp
{
    class PacketPlayerMenu : public PlayerPacket
    {
    public:
        PacketPlayerMenu(RakNet::RakPeerInterface *peer);

        virtual void Packet(RakNet::BitStream *newBitstream, bool send);
    };
}

#endif //OPENMW_PACKETPLAYERMENU_HPP
