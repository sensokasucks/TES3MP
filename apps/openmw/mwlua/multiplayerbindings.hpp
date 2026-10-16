#ifndef MWLUA_MULTIPLAYERBINDINGS_H
#define MWLUA_MULTIPLAYERBINDINGS_H

#include <sol/forward.hpp>

namespace MWLua
{
    struct Context;

    // openmw.multiplayer: raw event channel to the TES3MP server (player scripts only)
    sol::table initMultiplayerPackage(const Context& context);
}

#endif // MWLUA_MULTIPLAYERBINDINGS_H
