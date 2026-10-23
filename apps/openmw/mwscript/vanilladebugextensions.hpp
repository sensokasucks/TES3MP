#ifndef GAME_SCRIPT_VANILLADEBUGEXTENSIONS_H
#define GAME_SCRIPT_VANILLADEBUGEXTENSIONS_H

namespace Interpreter
{
    class Interpreter;
}

/*
    Start of tes3mp addition

    Console commands from the original Morrowind.exe that OpenMW did not have
*/
namespace MWScript
{
    namespace VanillaDebug
    {
        void installOpcodes(Interpreter::Interpreter& interpreter);
    }
}
/*
    End of tes3mp addition
*/

#endif
