#include "vanilladebug.hpp"

#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

#include <osg/Image>
#include <osgDB/Registry>

#include <components/debug/debuglog.hpp>
#include <components/esm3/loadcell.hpp>
#include <components/fallback/fallback.hpp>
#include <components/files/conversion.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/windowmanager.hpp"
#include "../mwbase/world.hpp"

#include "../mwmp/LuaChannel.hpp"

#include "actionteleport.hpp"
#include "cellstore.hpp"
#include "class.hpp"
#include "esmstore.hpp"
#include "player.hpp"
#include "worldmodel.hpp"

namespace
{
    unsigned sFlags = 0;
    std::filesystem::path sUserDataPath;

    // Frames to wait in each cell during a cell walk, so that the local map is rendered and the global
    // map has copied its tile back from the GPU (GlobalMap::copyResult waits two frames itself)
    constexpr int sFramesPerCell = 12;

    struct CellWalk
    {
        bool mActive = false;
        bool mExportMaps = false;
        std::vector<std::pair<int, int>> mCells;
        std::size_t mNext = 0;
        int mFramesLeft = 0;
        ESM::RefId mReturnCell;
        ESM::Position mReturnPosition{};
    };

    CellWalk sWalk;

    std::string jsonEscape(std::string_view text)
    {
        std::string result;
        result.reserve(text.size() + 2);
        for (char c : text)
        {
            switch (c)
            {
                case '"':
                    result += "\\\"";
                    break;
                case '\\':
                    result += "\\\\";
                    break;
                case '\n':
                    result += "\\n";
                    break;
                case '\r':
                    result += "\\r";
                    break;
                case '\t':
                    result += "\\t";
                    break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20)
                    {
                        std::ostringstream escaped;
                        escaped << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(c);
                        result += escaped.str();
                    }
                    else
                        result += c;
            }
        }
        return result;
    }

    // Cell names can contain characters Windows does not allow in file names
    std::string toFileName(std::string_view name)
    {
        std::string result;
        for (char c : name)
        {
            if (c == '<' || c == '>' || c == ':' || c == '"' || c == '/' || c == '\\' || c == '|' || c == '?'
                || c == '*' || static_cast<unsigned char>(c) < 0x20)
                result += '_';
            else
                result += c;
        }
        return result.empty() ? "Wilderness" : result;
    }

    std::filesystem::path getMapsDirectory()
    {
        std::filesystem::path dir = sUserDataPath / "maps";
        std::error_code ec;
        std::filesystem::create_directories(dir, ec);
        return dir;
    }

    bool writePng(const osg::Image& image, const std::filesystem::path& path)
    {
        osgDB::ReaderWriter* writer = osgDB::Registry::instance()->getReaderWriterForExtension("png");
        if (!writer)
        {
            Log(Debug::Error) << "Can't write " << path << ": no png writer";
            return false;
        }

        std::ofstream stream(path, std::ios::binary);
        osgDB::ReaderWriter::WriteResult result = writer->writeImage(image, stream);
        if (!result.success())
        {
            Log(Debug::Error) << "Can't write " << path << ": " << result.message();
            return false;
        }
        return true;
    }

    std::string describeCell(const MWWorld::CellStore* cell)
    {
        if (cell == nullptr)
            return "NO CELL";

        std::string name{ MWBase::Environment::get().getWorld()->getCellName(cell) };
        if (cell->isExterior())
            name += " (" + std::to_string(cell->getCell()->getGridX()) + ", "
                + std::to_string(cell->getCell()->getGridY()) + ")";
        return name;
    }

    std::filesystem::path getBetaCommentPath()
    {
        // Same Morrowind.ini key as vanilla, imported by mwiniimporter as a fallback
        std::string_view configured = Fallback::Map::getString("General_Beta_Comment_File");
        if (configured.empty())
            return sUserDataPath / "betacomments.txt";

        std::filesystem::path path = Files::pathFromUnicodeString(configured);
        if (path.is_relative())
            path = sUserDataPath / path;
        return path;
    }

    void finishCellWalk()
    {
        const bool exported = sWalk.mExportMaps;
        sWalk.mActive = false;
        sWalk.mExportMaps = false;

        MWBase::World* world = MWBase::Environment::get().getWorld();
        if (!sWalk.mReturnCell.empty())
        {
            MWWorld::Ptr player = world->getPlayerPtr();
            MWWorld::ActionTeleport(sWalk.mReturnCell, sWalk.mReturnPosition, false).execute(player);
        }

        if (exported)
        {
            const std::filesystem::path dir = getMapsDirectory();
            MWBase::Environment::get().getWindowManager()->exportWorldMap(dir);
            MWWorld::VanillaDebug::print("CreateMaps finished. Maps written to " + Files::pathToUnicodeString(dir));
        }
        else
            MWWorld::VanillaDebug::print("Cell test finished.");
    }
}

namespace MWWorld::VanillaDebug
{
    bool isEnabled(Flag flag)
    {
        return (sFlags & flag) != 0;
    }

    void setEnabled(Flag flag, bool enabled)
    {
        if (enabled)
            sFlags |= flag;
        else
            sFlags &= ~static_cast<unsigned>(flag);
    }

    bool toggle(Flag flag)
    {
        setEnabled(flag, !isEnabled(flag));
        return isEnabled(flag);
    }

    std::string_view onOff(bool value)
    {
        return value ? "On" : "Off";
    }

    std::string describe(const MWWorld::Ptr& ptr)
    {
        if (ptr.isEmpty())
            return "nothing";
        std::string_view name = ptr.getClass().getName(ptr);
        if (!name.empty())
            return std::string(name);
        return ptr.getCellRef().getRefId().toString();
    }

    std::string format(const char* fmt, ...)
    {
        char buffer[1024];
        va_list args;
        va_start(args, fmt);
        std::vsnprintf(buffer, sizeof(buffer), fmt, args);
        va_end(args);
        return buffer;
    }

    void print(std::string_view message)
    {
        Log(Debug::Info) << "[Vanilla debug] " << message;

        MWBase::WindowManager* windowManager = MWBase::Environment::get().getWindowManager();
        if (windowManager != nullptr)
            windowManager->printToConsole(std::string(message), MWBase::WindowManager::sConsoleColor_Info);
    }

    void setUserDataPath(const std::filesystem::path& path)
    {
        sUserDataPath = path;
    }

    const std::filesystem::path& getUserDataPath()
    {
        return sUserDataPath;
    }

    std::string writeBetaComment(const MWWorld::Ptr& ptr, std::string_view comment)
    {
        std::time_t now = std::time(nullptr);
        std::tm local{};
#ifdef _WIN32
        localtime_s(&local, &now);
#else
        localtime_r(&now, &local);
#endif
        std::ostringstream time;
        time << std::put_time(&local, "%m/%d/%Y %H:%M");

        const ESM::Position& pos = ptr.getRefData().getPosition();
        const std::string refId = ptr.getCellRef().getRefId().toString();
        const std::string cell = describeCell(ptr.getCell());

        // Same fields and order as the vanilla Beta Comment File, tab separated
        std::ostringstream line;
        line << std::fixed << std::setprecision(0) << time.str() << '\t' << refId << '\t' << cell << '\t' << pos.pos[0]
             << '\t' << pos.pos[1] << '\t' << pos.pos[2] << '\t' << comment;

        const std::filesystem::path path = getBetaCommentPath();
        {
            std::error_code ec;
            std::filesystem::create_directories(path.parent_path(), ec);
            std::ofstream file(path, std::ios::app);
            if (file)
                file << line.str() << '\n';
            else
                Log(Debug::Error) << "Can't open beta comment file " << path;
        }

        // Also report it to the server, where hooks/betaComments.lua logs it with the player's name
        std::ostringstream json;
        json << std::fixed << std::setprecision(2) << "{\"refId\":\"" << jsonEscape(refId)
             << "\",\"refNum\":" << ptr.getCellRef().getMpRefNum() << ",\"mpNum\":" << ptr.getCellRef().getMpNum()
             << ",\"cell\":\"" << jsonEscape(cell) << "\",\"x\":" << pos.pos[0] << ",\"y\":" << pos.pos[1]
             << ",\"z\":" << pos.pos[2] << ",\"comment\":\"" << jsonEscape(comment) << "\"}";
        mwmp::LuaChannel::pushOutgoing({ "betaComment", json.str() });

        return line.str();
    }

    void startCellWalk(bool exportMaps)
    {
        MWBase::World* world = MWBase::Environment::get().getWorld();
        MWWorld::Ptr player = world->getPlayerPtr();

        sWalk = CellWalk{};
        sWalk.mActive = true;
        sWalk.mExportMaps = exportMaps;
        if (player.isInCell())
        {
            sWalk.mReturnCell = player.getCell()->getCell()->getId();
            sWalk.mReturnPosition = player.getRefData().getPosition();
        }

        const MWWorld::Store<ESM::Cell>& cells = world->getStore().get<ESM::Cell>();
        for (auto it = cells.extBegin(); it != cells.extEnd(); ++it)
            sWalk.mCells.emplace_back(it->getGridX(), it->getGridY());

        if (exportMaps)
        {
            // Start a fresh index; writeMapTile appends to it
            std::ofstream index(getMapsDirectory() / "index.csv", std::ios::trunc);
            index << "x,y,name,file\n";
        }

        print(std::string(exportMaps ? "CreateMaps" : "Cell test") + ": visiting " + std::to_string(sWalk.mCells.size())
            + " exterior cells. Use StopCellTest (SCT) to stop.");
    }

    void stopCellWalk()
    {
        if (!sWalk.mActive)
            return;
        print("Stopping after " + std::to_string(sWalk.mNext) + " of " + std::to_string(sWalk.mCells.size())
            + " cells.");
        finishCellWalk();
    }

    bool isWalkingCells()
    {
        return sWalk.mActive;
    }

    bool isExportingMaps()
    {
        return sWalk.mActive && sWalk.mExportMaps;
    }

    void update(float /*duration*/)
    {
        if (!sWalk.mActive)
            return;

        if (sWalk.mFramesLeft > 0)
        {
            --sWalk.mFramesLeft;
            return;
        }

        if (sWalk.mNext >= sWalk.mCells.size())
        {
            finishCellWalk();
            return;
        }

        const auto [x, y] = sWalk.mCells[sWalk.mNext++];
        MWBase::Environment::get().getWindowManager()->executeCommandInConsole(
            "coe " + std::to_string(x) + " " + std::to_string(y));
        sWalk.mFramesLeft = sFramesPerCell;

        if (sWalk.mNext % 100 == 0)
            print("Cell test: " + std::to_string(sWalk.mNext) + " / " + std::to_string(sWalk.mCells.size()));
    }

    void writeMapTile(int cellX, int cellY, const osg::Image& image)
    {
        if (!isExportingMaps())
            return;

        const MWWorld::CellStore& cell = MWBase::Environment::get().getWorldModel()->getExterior(
            ESM::ExteriorCellLocation(cellX, cellY, ESM::Cell::sDefaultWorldspaceId));
        const std::string name{ MWBase::Environment::get().getWorld()->getCellName(&cell) };

        // Vanilla named these "Maps\<cell> (x,y).bmp"
        const std::string fileName
            = toFileName(name) + " (" + std::to_string(cellX) + "," + std::to_string(cellY) + ").png";
        const std::filesystem::path dir = getMapsDirectory();
        if (!writePng(image, dir / Files::pathFromUnicodeString(fileName)))
            return;

        std::ofstream index(dir / "index.csv", std::ios::app);
        std::string csvName = name;
        for (char& c : csvName)
            if (c == '"')
                c = '\'';
        index << cellX << ',' << cellY << ",\"" << csvName << "\",\"" << fileName << "\"\n";
    }
}
