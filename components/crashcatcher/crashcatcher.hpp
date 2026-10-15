#ifndef OPENMW_COMPONENTS_CRASHCATCHER_CRASHCATCHER_HPP
#define OPENMW_COMPONENTS_CRASHCATCHER_CRASHCATCHER_HPP

#include <filesystem>

/*
    Start of tes3mp change (major)

    Disable the crash catcher, which interferes with TES3MP's own logging and debugging
*/
#if (defined(__APPLE__) || (defined(__linux) && !defined(ANDROID)) || (defined(__unix) && !defined(ANDROID))           \
    || defined(__posix))
#define USE_CRASH_CATCHER 0
#else
#define USE_CRASH_CATCHER 0
#endif

#if USE_CRASH_CATCHER
void crashCatcherInstall(int argc, char** argv, const std::filesystem::path& crashLogPath);
#else
inline void crashCatcherInstall(int /*argc*/, char** /*argv*/, const std::filesystem::path& /*crashLogPath*/) {}
#endif
/*
    End of tes3mp change (major)
*/

#endif
