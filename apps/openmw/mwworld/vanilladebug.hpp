#ifndef GAME_MWWORLD_VANILLADEBUG_H
#define GAME_MWWORLD_VANILLADEBUG_H

#include <filesystem>
#include <string>
#include <string_view>

namespace osg
{
    class Image;
}

namespace MWWorld
{
    class Ptr;
}

/*
    Start of tes3mp addition

    Debug tools from the original Morrowind.exe that OpenMW did not have: the TCS/TKS/TMS/TDS stats logs,
    ToggleDebugText, ToggleTextureString, ToggleScriptOutput, MoveOneToOne, ToggleLoadFade, BetaComment
    reports, and the CreateMaps / TestThreadCells cell walk. The console commands live in
    mwscript/vanilladebugextensions.cpp; this file holds the state they share with the rest of the engine.
*/
namespace MWWorld::VanillaDebug
{
    enum Flag : unsigned
    {
        Flag_CombatStats = 1 << 0,
        Flag_MagicStats = 1 << 1,
        Flag_DialogueStats = 1 << 2,
        Flag_KillStats = 1 << 3,
        Flag_ScriptOutput = 1 << 4,
        Flag_DebugText = 1 << 5,
        Flag_TextureString = 1 << 6,
        Flag_MoveOneToOne = 1 << 7,
        Flag_NoLoadFade = 1 << 8,
    };

    bool isEnabled(Flag flag);
    void setEnabled(Flag flag, bool enabled);
    // Returns the new state
    bool toggle(Flag flag);

    // Writes a line to the console (when it exists) and to openmw.log, like the vanilla stats logs did
    void print(std::string_view message);

    void setUserDataPath(const std::filesystem::path& path);
    const std::filesystem::path& getUserDataPath();

    // BetaComment: appends a report about ptr to the beta comment file and sends it to the server.
    // Returns the report text.
    std::string writeBetaComment(const MWWorld::Ptr& ptr, std::string_view comment);

    // CreateMaps / TestThreadCells: visit every exterior cell, one per few frames, so that the local and
    // global maps get rendered for each. With exportMaps, every global map tile is also written to
    // <user data>/maps as a PNG, and the whole world map at the end.
    void startCellWalk(bool exportMaps);
    void stopCellWalk();
    bool isWalkingCells();
    bool isExportingMaps();

    // Called once per frame from World::update
    void update(float duration);

    // Called by the global map when a tile has been copied back from the GPU
    void writeMapTile(int cellX, int cellY, const osg::Image& image);

    // "On"/"Off", as the vanilla toggles print it
    std::string_view onOff(bool value);

    // An object's display name, or its ID when it has none, for the stats logs
    std::string describe(const MWWorld::Ptr& ptr);

    // printf-style formatting for the stats logs, which keep the vanilla message formats
    std::string format(const char* fmt, ...);
}
/*
    End of tes3mp addition
*/

#endif
