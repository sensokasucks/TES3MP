#ifndef GAME_MWMECHANICS_ACTIVESPELLS_H
#define GAME_MWMECHANICS_ACTIVESPELLS_H

#include <functional>
#include <list>
#include <queue>
#include <string>
#include <variant>
#include <vector>

#include <components/esm3/activespells.hpp>

#include "../mwworld/ptr.hpp"
#include "../mwworld/timestamp.hpp"

#include "spellcasting.hpp"

namespace ESM
{
    struct Enchantment;
    struct Spell;
}

namespace MWMechanics
{
    /// \brief Lasting spell effects
    ///
    /// \note The name of this class is slightly misleading, since it also handles lasting potion
    /// effects.
    class ActiveSpells
    {
    public:
        using ActiveEffect = ESM::ActiveEffect;
        class ActiveSpellParams
        {
            ESM::RefId mActiveSpellId;
            ESM::RefId mSourceSpellId;
            std::vector<ActiveEffect> mEffects;
            std::string mDisplayName;
            ESM::RefNum mCaster;
            ESM::RefNum mItem;
            ESM::ActiveSpells::Flags mFlags;
            int mWorsenings;
            MWWorld::TimeStamp mNextWorsening;
                    /*
                        Start of tes3mp addition

                        Track the timestamp at which this spell was added, so that stacked spells with the same id
                        can be told apart across clients, as well as whether this spell was received from the server
                        (in which case no packet should be sent back about it)
                    */
                    MWWorld::TimeStamp mTimeStamp;
                    bool mReceivedFromServer = false;
                    /*
                        End of tes3mp addition
                    */

            MWWorld::Ptr mSource;

            ActiveSpellParams(const ESM::ActiveSpells::ActiveSpellParams& params);

            ActiveSpellParams(const ESM::Spell* spell, const MWWorld::Ptr& actor, bool ignoreResistances = false);

            ActiveSpellParams(
                const MWWorld::ConstPtr& item, const ESM::Enchantment* enchantment, const MWWorld::Ptr& actor);
        public:
            /*
                Start of tes3mp change (minor)

                Make this constructor public so CastSpell::inflict can build reflected spells
            */

            ActiveSpellParams(const ActiveSpellParams& params, const MWWorld::Ptr& actor);
            /*
                End of tes3mp change (minor)
            */
        private:



            friend class ActiveSpells;

        public:
            ActiveSpellParams(
                const MWWorld::Ptr& caster, const ESM::RefId& id, std::string_view sourceName, ESM::RefNum item);

            /*
                Start of tes3mp addition

                Make it possible to create ActiveSpellParams from the ones received in a packet, to convert
                them back for packets, and to get their timestamp
            */
            ActiveSpellParams(const ESM::ActiveSpells::ActiveSpellParams& params, const ESM::RefNum& caster, MWWorld::TimeStamp timestamp);

            ESM::ActiveSpells::ActiveSpellParams toEsm() const;

            MWWorld::TimeStamp getTimeStamp() const { return mTimeStamp; }
            /*
                End of tes3mp addition
            */

            ESM::RefId getActiveSpellId() const { return mActiveSpellId; }
            void setActiveSpellId(ESM::RefId id) { mActiveSpellId = id; }

            const ESM::RefId& getSourceSpellId() const { return mSourceSpellId; }

            const std::vector<ActiveEffect>& getEffects() const { return mEffects; }
            std::vector<ActiveEffect>& getEffects() { return mEffects; }

            ESM::RefNum getCaster() const { return mCaster; }

            int getWorsenings() const { return mWorsenings; }

            const std::string& getDisplayName() const { return mDisplayName; }

            ESM::RefNum getItem() const { return mItem; }
            ESM::RefId getEnchantment() const;

            const ESM::Spell* getSpell() const;
            bool hasFlag(ESM::ActiveSpells::Flags flags) const;
            void setFlag(ESM::ActiveSpells::Flags flags);

            // Increments worsenings count and sets the next timestamp
            void worsen();

            bool shouldWorsen() const;

            void resetWorsenings();
        };

        typedef std::list<ActiveSpellParams> Collection;
        typedef Collection::const_iterator TIterator;

        void readState(const ESM::ActiveSpells& state);
        void writeState(ESM::ActiveSpells& state) const;

        TIterator begin() const;

        TIterator end() const;

        TIterator getActiveSpellById(const ESM::RefId& id);

        void update(const MWWorld::Ptr& ptr, float duration);

    private:
        using ParamsPredicate = std::function<bool(const ActiveSpellParams&)>;
        using EffectPredicate = std::function<bool(const ActiveSpellParams&, const ESM::ActiveEffect&)>;
        using Predicate = std::variant<ParamsPredicate, EffectPredicate>;

        struct IterationGuard
        {
            ActiveSpells& mActiveSpells;

            IterationGuard(ActiveSpells& spells);
            ~IterationGuard();
        };
            /*
                Start of tes3mp addition

                Allow the temporary suppression of packets, for removals requested by the server, and
                declare helper methods for sending packets about additions and removals of active spells
            */
            bool mSuppressPackets = false;

            void sendAddition(const MWWorld::Ptr& ptr, const ActiveSpellParams& params) const;
            void sendRemoval(const MWWorld::Ptr& ptr, const ActiveSpellParams& params) const;
            /*
                End of tes3mp addition
            */

        struct UpdateContext;

        std::list<ActiveSpellParams> mSpells;
        std::vector<ActiveSpellParams> mQueue;
        std::queue<Predicate> mPurges;
        bool mIterating;

        void addToSpells(const MWWorld::Ptr& ptr, const ActiveSpellParams& spell, UpdateContext& context);

        bool applyPurges(const MWWorld::Ptr& ptr, std::list<ActiveSpellParams>::iterator* currentSpell = nullptr,
            std::vector<ActiveEffect>::iterator* currentEffect = nullptr);

        bool updateActiveSpell(
            const MWWorld::Ptr& ptr, float duration, Collection::iterator& spellIt, UpdateContext& context);

        bool initParams(const MWWorld::Ptr& ptr, const ActiveSpellParams& params, UpdateContext& context);

    public:
        ActiveSpells();

        /// Add lasting effects
        ///
        /// \brief addSpell
        /// \param id ID for stacking purposes.
        ///
        void addSpell(const ActiveSpellParams& params);

        /// Force resistances
        void addSpell(const ESM::Spell* spell, const MWWorld::Ptr& actor, bool ignoreResistances = true);
            /*
                Start of tes3mp addition

                Add a separate addSpell() for spells received from the server, which get the timestamp
                they had on the client that sent them and do not get sent back to the server
            */
            void addSpell(const ESM::ActiveSpells::ActiveSpellParams& params, const ESM::RefNum& caster, MWWorld::TimeStamp timestamp);
            /*
                End of tes3mp addition
            */


        /// Removes the active effects from this spell/potion/.. with \a id
        void removeEffectsBySourceSpellId(const MWWorld::Ptr& ptr, const ESM::RefId& id);
        /// Removes the active effects of a specific active spell
        void removeEffectsByActiveSpellId(const MWWorld::Ptr& ptr, const ESM::RefId& id);

            /*
                Start of tes3mp addition

                Remove the spell with a certain ID and a certain timestamp, useful
                when there are stacked spells with the same ID
            */
            bool removeSpellByTimestamp(const MWWorld::Ptr& ptr, const std::string& id, MWWorld::TimeStamp timestamp);

            /*
                Remove all spells with a certain ID without sending packets about them, for removals
                requested by the server
            */
            void removeSpellsById(const MWWorld::Ptr& ptr, const std::string& id);
            /*
                End of tes3mp addition
            */

        /// Remove all active effects with this effect id
        void purgeEffect(const MWWorld::Ptr& ptr, ESM::RefId effectId, ESM::RefId effectArg = {});

        void purge(EffectPredicate predicate, const MWWorld::Ptr& ptr);
        void purge(ParamsPredicate predicate, const MWWorld::Ptr& ptr);

        /// Remove all effects that were cast by \a actor
        void purge(const MWWorld::Ptr& ptr, ESM::RefNum actor);
            /*
                Start of tes3mp addition

                Allow the purging of an effect for a specific arg (attribute or skill)
            */
            void purgeEffectByArg(const MWWorld::Ptr& ptr, const ESM::RefId& effectId, int effectArg);
            /*
                End of tes3mp addition
            */

            /*
                Start of tes3mp addition

                Make it easy to get an effect's duration
            */
            float getEffectDuration(const ESM::RefId& effectId, const std::string& sourceId) const;
            /*
                End of tes3mp addition
            */


        /// Remove all spells
        void clear(const MWWorld::Ptr& ptr);

        /// True if a spell associated with this id is active
        /// \note For enchantments, this is the id of the enchanted item, not the enchantment itself
        bool isSpellActive(const ESM::RefId& id) const;

        /// True if the enchantment is active
        bool isEnchantmentActive(const ESM::RefId& id) const;

        void skipWorsenings(double hours);


        void unloadActor(const MWWorld::Ptr& ptr);
    };
}

#endif
