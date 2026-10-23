#include "vanilladebugextensions.hpp"

#include <set>
#include <sstream>
#include <vector>

#include <components/compiler/opcodes.hpp>
#include <components/esm/records.hpp>
#include <components/interpreter/interpreter.hpp>
#include <components/interpreter/opcodes.hpp>
#include <components/interpreter/runtime.hpp>
#include <components/resource/resourcesystem.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/mechanicsmanager.hpp"
#include "../mwbase/statemanager.hpp"
#include "../mwbase/world.hpp"

#include "../mwmechanics/aisequence.hpp"
#include "../mwmechanics/creaturestats.hpp"

#include "../mwrender/animation.hpp"

#include "../mwworld/cellstore.hpp"
#include "../mwworld/class.hpp"
#include "../mwworld/esmstore.hpp"
#include "../mwworld/scene.hpp"
#include "../mwworld/vanilladebug.hpp"

#include "interpretercontext.hpp"
#include "ref.hpp"

/*
    Start of tes3mp addition

    Console commands from the original Morrowind.exe that OpenMW did not have. The messages follow the
    vanilla wording ("Kill Stats -> On" and so on).
*/
namespace MWScript
{
    namespace VanillaDebug
    {
        using namespace MWWorld::VanillaDebug;

        void reportToggle(Interpreter::Runtime& runtime, std::string_view name, bool state)
        {
            runtime.getContext().report(std::string(name) + " -> " + std::string(onOff(state)));
        }

        template <Flag flag>
        class OpToggleFlag : public Interpreter::Opcode0
        {
            const char* mName;

        public:
            explicit OpToggleFlag(const char* name)
                : mName(name)
            {
            }

            void execute(Interpreter::Runtime& runtime) override { reportToggle(runtime, mName, toggle(flag)); }
        };

        class OpToggleCombatStats : public OpToggleFlag<Flag_CombatStats>
        {
        public:
            OpToggleCombatStats()
                : OpToggleFlag("Combat Stats")
            {
            }
        };

        class OpToggleKillStats : public OpToggleFlag<Flag_KillStats>
        {
        public:
            OpToggleKillStats()
                : OpToggleFlag("Kill Stats")
            {
            }
        };

        class OpToggleMagicStats : public OpToggleFlag<Flag_MagicStats>
        {
        public:
            OpToggleMagicStats()
                : OpToggleFlag("Magic Stats")
            {
            }
        };

        class OpToggleDialogueStats : public OpToggleFlag<Flag_DialogueStats>
        {
        public:
            OpToggleDialogueStats()
                : OpToggleFlag("Dialogue Stats")
            {
            }
        };

        class OpToggleDebugText : public OpToggleFlag<Flag_DebugText>
        {
        public:
            OpToggleDebugText()
                : OpToggleFlag("Debug Text")
            {
            }
        };

        class OpToggleTextureString : public OpToggleFlag<Flag_TextureString>
        {
        public:
            OpToggleTextureString()
                : OpToggleFlag("Texture String")
            {
            }
        };

        class OpToggleScriptOutput : public OpToggleFlag<Flag_ScriptOutput>
        {
        public:
            OpToggleScriptOutput()
                : OpToggleFlag("Script Output")
            {
            }
        };

        class OpMoveOneToOne : public OpToggleFlag<Flag_MoveOneToOne>
        {
        public:
            OpMoveOneToOne()
                : OpToggleFlag("Move one to one")
            {
            }
        };

        // Vanilla's flag means "fade on cell load", which is on by default
        class OpToggleLoadFade : public Interpreter::Opcode0
        {
        public:
            void execute(Interpreter::Runtime& runtime) override
            {
                reportToggle(runtime, "Load Fade", !toggle(Flag_NoLoadFade));
            }
        };

        // ToggleStats (TST) flips all four stats logs together, as in vanilla
        class OpToggleStats : public Interpreter::Opcode0
        {
        public:
            void execute(Interpreter::Runtime& runtime) override
            {
                reportToggle(runtime, "Combat Stats", toggle(Flag_CombatStats));
                reportToggle(runtime, "Magic Stats", toggle(Flag_MagicStats));
                reportToggle(runtime, "Dialogue Stats", toggle(Flag_DialogueStats));
                reportToggle(runtime, "Kill Stats", toggle(Flag_KillStats));
            }
        };

        // Vanilla's ToggleLights command does nothing; keep it so vanilla console habits don't error out
        class OpToggleLights : public Interpreter::Opcode0
        {
        public:
            void execute(Interpreter::Runtime& runtime) override
            {
                runtime.getContext().report("ToggleLights has no effect (it did nothing in Morrowind.exe either).");
            }
        };

        // Vanilla's grid display is closest to OpenMW's cell border rendering
        class OpToggleGrid : public Interpreter::Opcode0
        {
        public:
            void execute(Interpreter::Runtime& runtime) override
            {
                reportToggle(runtime, "Grid", MWBase::Environment::get().getWorld()->toggleBorders());
            }
        };

        class OpPurgeTextures : public Interpreter::Opcode0
        {
        public:
            void execute(Interpreter::Runtime& runtime) override
            {
                MWBase::Environment::get().getResourceSystem()->clearCache();
                runtime.getContext().report("Unused textures and models purged from the cache.");
            }
        };

        bool checkGameRunning(Interpreter::Runtime& runtime)
        {
            if (MWBase::Environment::get().getStateManager()->getState() == MWBase::StateManager::State_Running)
                return true;
            runtime.getContext().report("Load a game first.");
            return false;
        }

        template <bool exportMaps>
        class OpCellWalk : public Interpreter::Opcode0
        {
        public:
            void execute(Interpreter::Runtime& runtime) override
            {
                if (!checkGameRunning(runtime))
                    return;
                if (isWalkingCells())
                {
                    runtime.getContext().report("A cell test is already running. Use StopCellTest (SCT) first.");
                    return;
                }
                startCellWalk(exportMaps);
            }
        };

        class OpStopCellTest : public Interpreter::Opcode0
        {
        public:
            void execute(Interpreter::Runtime& runtime) override
            {
                if (!isWalkingCells())
                    runtime.getContext().report("No cell test is running.");
                stopCellWalk();
            }
        };

        class OpOutputObjCounts : public Interpreter::Opcode0
        {
            template <class T>
            static void line(Interpreter::Runtime& runtime, const MWWorld::ESMStore& store, const char* name)
            {
                runtime.getContext().report(std::string(name) + ": " + std::to_string(store.get<T>().getSize()));
            }

        public:
            void execute(Interpreter::Runtime& runtime) override
            {
                const MWWorld::ESMStore& store = MWBase::Environment::get().getWorld()->getStore();
                line<ESM::Activator>(runtime, store, "Activators");
                line<ESM::Apparatus>(runtime, store, "Apparatus");
                line<ESM::Armor>(runtime, store, "Armor");
                line<ESM::BodyPart>(runtime, store, "Body parts");
                line<ESM::Book>(runtime, store, "Books");
                line<ESM::BirthSign>(runtime, store, "Birthsigns");
                line<ESM::Cell>(runtime, store, "Cells");
                line<ESM::Class>(runtime, store, "Classes");
                line<ESM::Clothing>(runtime, store, "Clothing");
                line<ESM::Container>(runtime, store, "Containers");
                line<ESM::Creature>(runtime, store, "Creatures");
                line<ESM::Dialogue>(runtime, store, "Dialogue topics");
                line<ESM::Door>(runtime, store, "Doors");
                line<ESM::Enchantment>(runtime, store, "Enchantments");
                line<ESM::Faction>(runtime, store, "Factions");
                line<ESM::GameSetting>(runtime, store, "Game settings");
                line<ESM::Global>(runtime, store, "Globals");
                line<ESM::Ingredient>(runtime, store, "Ingredients");
                line<ESM::Land>(runtime, store, "Landscape");
                line<ESM::CreatureLevList>(runtime, store, "Leveled creatures");
                line<ESM::ItemLevList>(runtime, store, "Leveled items");
                line<ESM::Light>(runtime, store, "Lights");
                line<ESM::Lockpick>(runtime, store, "Lockpicks");
                line<ESM::Miscellaneous>(runtime, store, "Misc items");
                line<ESM::NPC>(runtime, store, "NPCs");
                line<ESM::Potion>(runtime, store, "Potions");
                line<ESM::Probe>(runtime, store, "Probes");
                line<ESM::Race>(runtime, store, "Races");
                line<ESM::Region>(runtime, store, "Regions");
                line<ESM::Repair>(runtime, store, "Repair items");
                line<ESM::Script>(runtime, store, "Scripts");
                line<ESM::Sound>(runtime, store, "Sounds");
                line<ESM::Spell>(runtime, store, "Spells");
                line<ESM::Static>(runtime, store, "Statics");
                line<ESM::Weapon>(runtime, store, "Weapons");
            }
        };

        class OpOutputRefCounts : public Interpreter::Opcode0
        {
        public:
            void execute(Interpreter::Runtime& runtime) override
            {
                MWBase::World* world = MWBase::Environment::get().getWorld();
                std::size_t total = 0;
                std::size_t totalTemp = 0;
                for (MWWorld::CellStore* cell : MWBase::Environment::get().getWorldScene()->getActiveCells())
                {
                    std::size_t refs = 0;
                    std::size_t temp = 0;
                    cell->forEach([&](const MWWorld::Ptr& ptr) {
                        ++refs;
                        // References created at runtime (server spawns, dropped items) have no content file
                        if (!ptr.getCellRef().hasContentFile())
                            ++temp;
                        return true;
                    });
                    total += refs;
                    totalTemp += temp;

                    std::ostringstream line;
                    line << world->getCellName(cell);
                    if (cell->isExterior())
                        line << " (" << cell->getCell()->getGridX() << ", " << cell->getCell()->getGridY() << ")";
                    line << ": " << refs << " references, " << temp << " temprefs";
                    runtime.getContext().report(line.str());
                }
                runtime.getContext().report("Total references in active cells:\t" + std::to_string(total));
                runtime.getContext().report("...that are temprefs:\t\t" + std::to_string(totalTemp));
            }
        };

        template <class R>
        class OpShowTargets : public Interpreter::Opcode0
        {
        public:
            void execute(Interpreter::Runtime& runtime) override
            {
                MWWorld::Ptr ptr = R()(runtime);
                if (!ptr.getClass().isActor())
                {
                    runtime.getContext().report(describe(ptr) + " is not an actor.");
                    return;
                }

                std::vector<MWWorld::Ptr> targets;
                ptr.getClass().getCreatureStats(ptr).getAiSequence().getCombatTargets(targets);
                runtime.getContext().report("Targets of " + describe(ptr) + ":");
                if (targets.empty())
                    runtime.getContext().report("  none");
                for (const MWWorld::Ptr& target : targets)
                    runtime.getContext().report("  " + describe(target));
            }
        };

        template <class R>
        class OpShowGroup : public Interpreter::Opcode0
        {
        public:
            void execute(Interpreter::Runtime& runtime) override
            {
                MWWorld::Ptr ptr = R()(runtime);
                if (!ptr.getClass().isActor())
                {
                    runtime.getContext().report(describe(ptr) + " is not an actor.");
                    return;
                }

                std::set<MWWorld::Ptr> group;
                MWBase::Environment::get().getMechanicsManager()->getActorsSidingWith(ptr, group);
                runtime.getContext().report("Group of " + describe(ptr) + ":");
                if (group.empty())
                    runtime.getContext().report("  none");
                for (const MWWorld::Ptr& member : group)
                    runtime.getContext().report("  " + describe(member));
            }
        };

        template <class R>
        class OpShowAnim : public Interpreter::Opcode0
        {
        public:
            void execute(Interpreter::Runtime& runtime) override
            {
                MWWorld::Ptr ptr = R()(runtime);
                const MWRender::Animation* animation = MWBase::Environment::get().getWorld()->getAnimation(ptr);
                if (animation == nullptr)
                {
                    runtime.getContext().report(describe(ptr) + " has no animation.");
                    return;
                }

                static const char* const boneGroups[]
                    = { "Lower body", "Torso", "Left arm", "Right arm" };
                runtime.getContext().report("Animation of " + describe(ptr) + ":");
                for (int i = 0; i < MWRender::Num_BoneGroups; ++i)
                {
                    const std::string_view group = animation->getActiveGroup(static_cast<MWRender::BoneGroup>(i));
                    if (group.empty())
                        continue;
                    runtime.getContext().report(format("  %s: %s  time %.2f  velocity %.2f", boneGroups[i],
                        std::string(group).c_str(), animation->getCurrentTime(group), animation->getVelocity(group)));
                }
            }
        };

        void installOpcodes(Interpreter::Interpreter& interpreter)
        {
            interpreter.installSegment5<OpToggleCombatStats>(Compiler::VanillaDebug::opcodeToggleCombatStats);
            interpreter.installSegment5<OpToggleKillStats>(Compiler::VanillaDebug::opcodeToggleKillStats);
            interpreter.installSegment5<OpToggleMagicStats>(Compiler::VanillaDebug::opcodeToggleMagicStats);
            interpreter.installSegment5<OpToggleDialogueStats>(Compiler::VanillaDebug::opcodeToggleDialogueStats);
            interpreter.installSegment5<OpToggleStats>(Compiler::VanillaDebug::opcodeToggleStats);
            interpreter.installSegment5<OpToggleDebugText>(Compiler::VanillaDebug::opcodeToggleDebugText);
            interpreter.installSegment5<OpToggleTextureString>(Compiler::VanillaDebug::opcodeToggleTextureString);
            interpreter.installSegment5<OpToggleScriptOutput>(Compiler::VanillaDebug::opcodeToggleScriptOutput);
            interpreter.installSegment5<OpMoveOneToOne>(Compiler::VanillaDebug::opcodeMoveOneToOne);
            interpreter.installSegment5<OpToggleLoadFade>(Compiler::VanillaDebug::opcodeToggleLoadFade);
            interpreter.installSegment5<OpPurgeTextures>(Compiler::VanillaDebug::opcodePurgeTextures);
            interpreter.installSegment5<OpToggleGrid>(Compiler::VanillaDebug::opcodeToggleGrid);
            interpreter.installSegment5<OpToggleLights>(Compiler::VanillaDebug::opcodeToggleLights);
            interpreter.installSegment5<OpCellWalk<true>>(Compiler::VanillaDebug::opcodeCreateMaps);
            interpreter.installSegment5<OpCellWalk<false>>(Compiler::VanillaDebug::opcodeTestThreadCells);
            interpreter.installSegment5<OpStopCellTest>(Compiler::VanillaDebug::opcodeStopCellTest);
            interpreter.installSegment5<OpOutputObjCounts>(Compiler::VanillaDebug::opcodeOutputObjCounts);
            interpreter.installSegment5<OpOutputRefCounts>(Compiler::VanillaDebug::opcodeOutputRefCounts);
            interpreter.installSegment5<OpShowTargets<ImplicitRef>>(Compiler::VanillaDebug::opcodeShowTargets);
            interpreter.installSegment5<OpShowTargets<ExplicitRef>>(Compiler::VanillaDebug::opcodeShowTargetsExplicit);
            interpreter.installSegment5<OpShowGroup<ImplicitRef>>(Compiler::VanillaDebug::opcodeShowGroup);
            interpreter.installSegment5<OpShowGroup<ExplicitRef>>(Compiler::VanillaDebug::opcodeShowGroupExplicit);
            interpreter.installSegment5<OpShowAnim<ImplicitRef>>(Compiler::VanillaDebug::opcodeShowAnim);
            interpreter.installSegment5<OpShowAnim<ExplicitRef>>(Compiler::VanillaDebug::opcodeShowAnimExplicit);
        }
    }
}
/*
    End of tes3mp addition
*/
