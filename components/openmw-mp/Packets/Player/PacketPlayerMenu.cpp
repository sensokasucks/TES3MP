#include <components/openmw-mp/NetworkMessages.hpp>
#include "PacketPlayerMenu.hpp"

using namespace mwmp;

PacketPlayerMenu::PacketPlayerMenu(RakNet::RakPeerInterface *peer) : PlayerPacket(peer)
{
    packetID = ID_PLAYER_MENU;
}

void PacketPlayerMenu::Packet(RakNet::BitStream *newBitstream, bool send)
{
    PlayerPacket::Packet(newBitstream, send);

    RW(player->menu, send, true);
    RW(player->menuTargetRefNum, send);
    RW(player->menuTargetMpNum, send);

    if (!send && player->menu.size() > 64)
        packetValid = false;
}
