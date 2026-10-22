#include <MyGUI_ScrollBar.h>

/*
    Start of tes3mp addition

    Include additional headers for multiplayer purposes
*/
#include "../mwmp/Main.hpp"
#include "../mwmp/LocalPlayer.hpp"
/*
    End of tes3mp addition
*/

#include <components/misc/rng.hpp>
#include <components/misc/strings/format.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/luamanager.hpp"
#include "../mwbase/mechanicsmanager.hpp"
#include "../mwbase/windowmanager.hpp"
#include "../mwbase/world.hpp"

#include "../mwmechanics/actorutil.hpp"
#include "../mwmechanics/npcstats.hpp"

#include "../mwworld/class.hpp"
#include "../mwworld/esmstore.hpp"
#include "../mwworld/store.hpp"

#include "jailscreen.hpp"

namespace MWGui
{
    JailScreen::JailScreen()
        : WindowBase("openmw_jail_screen.layout")
        , mDays(1)
        , mFadeTimeRemaining(0)
    {
        getWidget(mProgressBar, "ProgressBar");

        mTimeAdvancer.eventProgressChanged += MyGUI::newDelegate(this, &JailScreen::onJailProgressChanged);
        mTimeAdvancer.eventFinished += MyGUI::newDelegate(this, &JailScreen::onJailFinished);

        center();
    }

    void JailScreen::goToJail(int days)
    {
        mDays = days;

        MWBase::Environment::get().getWindowManager()->fadeScreenOut(0.5);
        mFadeTimeRemaining = 0.5;

        setVisible(false);
        mProgressBar->setScrollRange(100 + 1);
        mProgressBar->setScrollPosition(0);
        mProgressBar->setTrackSize(0);

        /*
            Start of tes3mp addition

            If we've received a packet overriding the default jail progress text, use the new text
        */
        if (!mwmp::Main::get().getLocalPlayer()->jailProgressText.empty())
            setText("LoadingText", mwmp::Main::get().getLocalPlayer()->jailProgressText);
        /*
            End of tes3mp addition
        */
    }

    void JailScreen::onFrame(float dt)
    {
        mTimeAdvancer.onFrame(dt);

        if (mFadeTimeRemaining <= 0)
            return;

        mFadeTimeRemaining -= dt;

        if (mFadeTimeRemaining <= 0)
        {
            MWWorld::Ptr player = MWMechanics::getPlayer();

            /*
                Start of tes3mp change (minor)

                Prevent teleportation to jail if specified
            */
            if (!mwmp::Main::get().getLocalPlayer()->ignoreJailTeleportation)
            {
            MWBase::Environment::get().getWorld()->teleportToClosestMarker(
                player, ESM::RefId::stringRefId("prisonmarker"));
            MWBase::Environment::get().getWindowManager()->fadeScreenOut(
                0.f); // override fade-in caused by cell transition
            }
            /*
                End of tes3mp change (minor)
            */

            setVisible(true);
            mTimeAdvancer.run(100);
        }
    }

    void JailScreen::onJailProgressChanged(int cur, int /*total*/)
    {
        mProgressBar->setScrollPosition(0);
        mProgressBar->setTrackSize(
            static_cast<int>(cur / (float)(mProgressBar->getScrollRange()) * mProgressBar->getLineSize()));
    }

    void JailScreen::onJailFinished()
    {
        MWBase::Environment::get().getWindowManager()->removeGuiMode(MWGui::GM_Jail);
        MWBase::Environment::get().getWindowManager()->fadeScreenIn(0.5);

        MWWorld::Ptr player = MWMechanics::getPlayer();
        /*
            Start of tes3mp addition

            Declare pointer to LocalPlayer for use in other additions
        */
        mwmp::LocalPlayer* localPlayer = mwmp::Main::get().getLocalPlayer();
        /*
            End of tes3mp addition
        */


        MWBase::Environment::get().getMechanicsManager()->rest(mDays * 24, true);

        /*
            Start of tes3mp change (major)

            Multiplayer requires that time not get advanced here
        */
        //MWBase::Environment::get().getWorld()->advanceTime(mDays * 24);
        /*
            End of tes3mp change (major)
        */

        // We should not worsen corprus when in prison
        player.getClass().getCreatureStats(player).getActiveSpells().skipWorsenings(mDays * 24);
        /*
            Start of tes3mp change (minor)

            Pass the PlayerJail overrides to the Lua jail handler, which applies the skill changes and shows the end
            message in 0.51; "%d" in a custom end text is replaced by the number of days, as in 0.8.1
        */
        std::string jailEndText = localPlayer->jailEndText;
        for (size_t pos = jailEndText.find("%d"); pos != std::string::npos; pos = jailEndText.find("%d", pos))
        {
            const std::string days = std::to_string(mDays);
            jailEndText.replace(pos, 2, days);
            pos += days.size();
        }
        MWBase::Environment::get().getLuaManager()->jailTimeServed(
            player, mDays, localPlayer->ignoreJailSkillIncreases, jailEndText);
        /*
            End of tes3mp change (minor)
        */

        /*
            Start of tes3mp addition

            Reset all PlayerJail-related overrides
        */
        localPlayer->ignoreJailTeleportation = false;
        localPlayer->ignoreJailSkillIncreases = false;
        localPlayer->jailProgressText = "";
        localPlayer->jailEndText = "";
        /*
            End of tes3mp addition
        */
    }
}
