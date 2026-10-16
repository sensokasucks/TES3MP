#include "LuaChannel.hpp"

#include <chrono>
#include <mutex>

#include <components/openmw-mp/TimedLog.hpp>
#include <components/openmw-mp/Packets/Player/PacketPlayerLuaEvent.hpp>

namespace
{
    std::mutex sMutex;
    std::vector<mwmp::LuaChannelEvent> sIncoming;
    std::vector<mwmp::LuaChannelEvent> sOutgoing;

    // Matches the server's limit so events aren't dropped silently on arrival
    const int sMaxOutgoingPerSecond = 30;
    const size_t sMaxQueued = 256;
    std::chrono::steady_clock::time_point sWindowStart;
    int sWindowCount = 0;
}

namespace mwmp
{
    void LuaChannel::pushIncoming(LuaChannelEvent event)
    {
        std::lock_guard<std::mutex> lock(sMutex);

        if (sIncoming.size() >= sMaxQueued)
        {
            LOG_MESSAGE_SIMPLE(TimedLog::LOG_WARN, "Dropping server Lua event %s because client scripts aren't reading them",
                event.name.c_str());
            return;
        }

        sIncoming.push_back(std::move(event));
    }

    std::vector<LuaChannelEvent> LuaChannel::takeOutgoing()
    {
        std::lock_guard<std::mutex> lock(sMutex);
        std::vector<LuaChannelEvent> result;
        result.swap(sOutgoing);
        return result;
    }

    bool LuaChannel::pushOutgoing(LuaChannelEvent event)
    {
        if (event.name.empty() || event.name.size() > PacketPlayerLuaEvent::maxNameLength
            || event.payload.size() > PacketPlayerLuaEvent::maxPayloadLength)
            return false;

        std::lock_guard<std::mutex> lock(sMutex);

        auto now = std::chrono::steady_clock::now();
        if (now - sWindowStart >= std::chrono::seconds(1))
        {
            sWindowStart = now;
            sWindowCount = 0;
        }

        if (sWindowCount >= sMaxOutgoingPerSecond || sOutgoing.size() >= sMaxQueued)
            return false;

        ++sWindowCount;
        sOutgoing.push_back(std::move(event));
        return true;
    }

    std::vector<LuaChannelEvent> LuaChannel::takeIncoming()
    {
        std::lock_guard<std::mutex> lock(sMutex);
        std::vector<LuaChannelEvent> result;
        result.swap(sIncoming);
        return result;
    }

    void LuaChannel::clear()
    {
        std::lock_guard<std::mutex> lock(sMutex);
        sIncoming.clear();
        sOutgoing.clear();
    }
}
