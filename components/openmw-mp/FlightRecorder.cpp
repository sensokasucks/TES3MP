#include "FlightRecorder.hpp"

#include <cstdlib>
#include <memory>

#include <BitStream.h>
#include <RakNetTypes.h>

#include "WindowsUndef.hpp"

#include "Flight/cofr.h"

namespace
{
    std::unique_ptr<cofr::Recorder> sRecorder;

    uint32_t peerId(const RakNet::RakNetGUID& guid)
    {
        return static_cast<uint32_t>(RakNet::RakNetGUID::ToUint32(guid));
    }

    // The first byte of every TES3MP and RakNet message is its message ID
    bool decode(const uint8_t* data, size_t size, cofr::NetMeta& out)
    {
        if (size == 0)
            return false;
        out.msgType = data[0];
        return true;
    }
}

namespace mwmp::Flight
{
    void init(const std::string& label, bool isServer)
    {
        const char* dir = std::getenv("TES3MP_FLIGHT_DIR");
        if (dir == nullptr || *dir == '\0')
            return;

        int slot = isServer ? 0 : 1;
        if (const char* slotText = std::getenv("TES3MP_FLIGHT_SLOT"))
            slot = std::atoi(slotText);

        cofr::Config config;
        config.dir = dir;
        config.label = label;
        config.slot = static_cast<uint8_t>(slot);
        config.role = isServer ? cofr::kRoleHost : cofr::kRoleClient;
        config.decoder = decode;
        sRecorder = std::make_unique<cofr::Recorder>(config);
        sRecorder->meta("program", label);
    }

    bool enabled()
    {
        return sRecorder != nullptr;
    }

    void netOut(const RakNet::RakNetGUID& to, bool broadcast, const RakNet::BitStream& stream, uint8_t channel)
    {
        if (!sRecorder)
            return;
        // A broadcast goes to everyone except `to`; record it under the reserved peer id 0xFFFFFFFF
        sRecorder->netOut(broadcast ? 0xFFFFFFFFu : peerId(to), stream.GetData(),
            static_cast<size_t>(stream.GetNumberOfBytesUsed()), channel);
    }

    void netIn(const RakNet::Packet& packet)
    {
        if (sRecorder)
            sRecorder->netIn(peerId(packet.guid), packet.data, packet.length);
    }

    void marker(const std::string& text)
    {
        if (sRecorder)
            sRecorder->marker(cofr::kMarkNote, text);
    }

    std::string dump(const char* reason)
    {
        if (!sRecorder)
            return std::string();
        sRecorder->marker(cofr::kMarkNote, std::string("dump: ") + reason);
        return sRecorder->dumpToDir(cofr::kDumpShutdown);
    }
}
