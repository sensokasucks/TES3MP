#ifndef OPENMW_LUACHANNEL_HPP
#define OPENMW_LUACHANNEL_HPP

#include <string>
#include <vector>

namespace mwmp
{
    struct LuaChannelEvent
    {
        std::string name;
        std::string payload;
    };

    // Thread-safe queues between client Lua (openmw.multiplayer) and the network code,
    // since Lua scripts may run on a worker thread
    class LuaChannel
    {
    public:
        // Network side
        static void pushIncoming(LuaChannelEvent event);
        static std::vector<LuaChannelEvent> takeOutgoing();

        // Lua side; returns false when the event is too large or the client is sending too many
        static bool pushOutgoing(LuaChannelEvent event);
        static std::vector<LuaChannelEvent> takeIncoming();

        static void clear();
    };
}

#endif //OPENMW_LUACHANNEL_HPP
