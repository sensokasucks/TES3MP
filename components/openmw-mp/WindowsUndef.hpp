#ifndef OPENMW_MP_WINDOWSUNDEF_HPP
#define OPENMW_MP_WINDOWSUNDEF_HPP

/*
    Start of tes3mp addition

    RakNet pulls in <windows.h>, whose windef.h defines `near` and `far` as empty macros.
    OpenMW uses those names as parameters (components/fx/stateupdater.hpp), so on MSVC this
    header is force-included (/FI) into every translation unit: it includes windows.h once,
    then removes the two macros before any OpenMW header is parsed.
*/
#ifdef _WIN32
#include <windows.h>
#undef near
#undef far
#endif
/*
    End of tes3mp addition
*/

#endif
