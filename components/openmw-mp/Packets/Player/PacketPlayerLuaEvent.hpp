#ifndef OPENMW_PACKETPLAYERLUAEVENT_HPP
#define OPENMW_PACKETPLAYERLUAEVENT_HPP

#include <components/openmw-mp/Packets/Player/PlayerPacket.hpp>

namespace mwmp
{
    // A named event with a text payload (JSON by convention) between server Lua and client Lua
    class PacketPlayerLuaEvent : public PlayerPacket
    {
    public:
        static const size_t maxNameLength = 64;
        static const size_t maxPayloadLength = 16384;

        PacketPlayerLuaEvent(RakNet::RakPeerInterface *peer);

        virtual void Packet(RakNet::BitStream *newBitstream, bool send);
    };
}

#endif //OPENMW_PACKETPLAYERLUAEVENT_HPP
