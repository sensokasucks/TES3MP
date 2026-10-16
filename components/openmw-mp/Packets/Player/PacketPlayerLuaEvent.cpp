#include <components/openmw-mp/NetworkMessages.hpp>
#include "PacketPlayerLuaEvent.hpp"

using namespace mwmp;

PacketPlayerLuaEvent::PacketPlayerLuaEvent(RakNet::RakPeerInterface *peer) : PlayerPacket(peer)
{
    packetID = ID_PLAYER_LUA_EVENT;
    orderChannel = CHANNEL_SYSTEM;
}

void PacketPlayerLuaEvent::Packet(RakNet::BitStream *newBitstream, bool send)
{
    PlayerPacket::Packet(newBitstream, send);

    RW(player->luaEventName, send, true);
    RW(player->luaEventPayload, send, true);

    if (!send && (player->luaEventName.empty() || player->luaEventName.size() > maxNameLength
        || player->luaEventPayload.size() > maxPayloadLength))
        packetValid = false;
}
