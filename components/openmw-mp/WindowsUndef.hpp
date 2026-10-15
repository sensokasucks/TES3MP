/*
    Start of tes3mp addition

    RakNet's headers pull in <windows.h>, whose macros clash with names OpenMW uses
    (`near`/`far` parameters, MWMechanics::DrawState, ...). Include this right after any
    RakNet header; it deliberately has no include guard so the macros are removed again
    wherever it is included.
*/
#ifdef _WIN32
#include <WinSock2.h>
#include <windows.h>
#undef near
#undef far
// NEAR/FAR expand to near/far, which were empty; keep later Windows SDK headers working
#undef NEAR
#undef FAR
#define NEAR
#define FAR
#undef DrawState
#endif
/*
    End of tes3mp addition
*/
