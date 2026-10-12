#ifndef GAME_MWDIALOG_JOURNAL_H
#define GAME_MWDIALOG_JOURNAL_H

#include "../mwbase/journal.hpp"

#include "quest.hpp"

namespace MWDialogue
{
    /// \brief The player's journal
    class Journal : public MWBase::Journal
    {
        TEntryContainer mJournal;
        TQuestContainer mQuests;
        TTopicContainer mTopics;

    private:
        Topic& getTopic(const ESM::RefId& id);

        bool isThere(const ESM::RefId& topicId, const ESM::RefId& infoId = ESM::RefId()) const;

    public:
        Journal();

        void clear() override;

        Quest* getQuestOrNull(const ESM::RefId& id) override;
        ///< Gets a pointer to the requested quest. Will return nullptr if the quest has not been started.

        Quest& getOrStartQuest(const ESM::RefId& id) override;
        ///< Gets the quest requested. Attempts to create it and inserts it in quests if it is not yet started.

        /*
            Start of tes3mp addition

            Make it possible to check whether a journal entry already exists from elsewhere in the code
        */
        bool hasEntry(const ESM::RefId& id, int index) override;
        /*
            End of tes3mp addition
        */

        /*
            Start of tes3mp change (minor)

            Make it possible to override current time when adding journal entries, by adding
            optional timestamp override arguments
        */
        void addEntry(const ESM::RefId& id, int index, const MWWorld::Ptr& actor, int daysPassed = -1, int month = -1, int day = -1) override;
        /*
            End of tes3mp change (minor)
        */
        ///< Add a journal entry.
        /// @param actor Used as context for replacing of escape sequences (%name, etc).
            /*
                End of tes3mp change (major)
            */

        void setJournalIndex(const ESM::RefId& id, int index) override;
        ///< Set the journal index without adding an entry.

        int getJournalIndex(const ESM::RefId& id) const override;
        ///< Get the journal index.

        void addTopic(const ESM::RefId& topicId, const ESM::RefId& infoId, const MWWorld::Ptr& actor) override;
        /// \note topicId must be lowercase

        void removeLastAddedTopicResponse(const ESM::RefId& topicId, std::string_view actorName) override;
        ///< Removes the last topic response added for the given topicId and actor name.
        /// \note topicId must be lowercase

        TEntryIter begin() const override;
        ///< Iterator pointing to the begin of the main journal.
        ///
        /// \note Iterators to main journal entries will never become invalid.

        TEntryIter end() const override;
        ///< Iterator pointing past the end of the main journal.

        TQuestIter questBegin() const override;
        ///< Iterator pointing to the first quest (sorted by topic ID)

        TQuestIter questEnd() const override;
        ///< Iterator pointing past the last quest.

        TTopicIter topicBegin() const override;
        ///< Iterator pointing to the first topic (sorted by topic ID)
        ///
        /// \note The topic ID is identical with the user-visible topic string.

        TTopicIter topicEnd() const override;
        ///< Iterator pointing past the last topic.

        int countSavedGameRecords() const override;

        void write(ESM::ESMWriter& writer, Loading::Listener& progress) const override;

        void readRecord(ESM::ESMReader& reader, uint32_t type) override;
    };
}

#endif
