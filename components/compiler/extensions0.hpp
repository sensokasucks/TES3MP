#ifndef COMPILER_EXTENSIONS0_H
#define COMPILER_EXTENSIONS0_H

namespace Compiler
{
    class Extensions;

    void registerExtensions(Extensions& extensions, bool consoleOnly = false);

    namespace Ai
    {
        void registerExtensions(Extensions& extensions);
    }

    namespace Animation
    {
        void registerExtensions(Extensions& extensions);
    }

    namespace Cell
    {
        void registerExtensions(Extensions& extensions);
    }

    namespace Console
    {
        void registerExtensions(Extensions& extensions);
    }

    namespace Container
    {
        void registerExtensions(Extensions& extensions);
    }

    namespace Control
    {
        void registerExtensions(Extensions& extensions);
    }

    namespace Dialogue
    {
        void registerExtensions(Extensions& extensions);
    }

    namespace Gui
    {
        void registerExtensions(Extensions& extensions);
    }

    namespace Misc
    {
        void registerExtensions(Extensions& extensions);
    }

    namespace Sky
    {
        void registerExtensions(Extensions& extensions);
    }

    namespace Sound
    {
        void registerExtensions(Extensions& extensions);
    }

    namespace Stats
    {
        void registerExtensions(Extensions& extensions);
    }

    namespace Transformation
    {
        void registerExtensions(Extensions& extensions);
    }

    namespace User
    {
        void registerExtensions(Extensions& extensions);
    }

    // tes3mp addition: console commands from the original Morrowind.exe. The short names are only
    // registered for the console, so that they can't clash with local variable names in scripts
    namespace VanillaDebug
    {
        void registerExtensions(Extensions& extensions, bool consoleOnly);
    }
}

#endif
