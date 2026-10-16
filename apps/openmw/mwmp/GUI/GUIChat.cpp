#include "GUIChat.hpp"

#include <algorithm>
#include <cctype>

#include <MyGUI_EditBox.h>
#include "apps/openmw/mwbase/environment.hpp"
#include "apps/openmw/mwgui/windowmanagerimp.hpp"
#include "apps/openmw/mwinput/inputmanagerimp.hpp"
#include <MyGUI_InputManager.h>
#include <components/openmw-mp/TimedLog.hpp>

#include "../Networking.hpp"
#include "../Main.hpp"
#include "../LocalPlayer.hpp"

#include "../GUIController.hpp"


namespace mwmp
{
    GUIChat::GUIChat(int x, int y, int w, int h)
            : WindowBase("tes3mp_chat.layout")
    {
        setCoord(x, y, w, h);

        getWidget(mCommandLine, "edit_Command");
        getWidget(mHistory, "list_History");
        getWidget(mSuggestionBox, "box_Suggestions");
        getWidget(mSuggestionList, "list_Suggestions");

        // Set up the command line box
        mCommandLine->eventEditSelectAccept +=
                newDelegate(this, &GUIChat::acceptCommand);
        mCommandLine->eventKeyButtonPressed +=
                newDelegate(this, &GUIChat::keyPress);
        mCommandLine->eventEditTextChange +=
                newDelegate(this, &GUIChat::textChanged);

        setTitle("Chat");

        mHistory->setOverflowToTheLeft(true);
        mHistory->setEditWordWrap(true);
        mHistory->setTextShadow(true);
        mHistory->setTextShadowColour(MyGUI::Colour::Black);

        mHistory->setNeedKeyFocus(false);

        windowState = CHAT_DISABLED;
        mCommandLine->setVisible(false);
        delay = 3; // 3 sec.
    }

    void GUIChat::onOpen()
    {
        // Give keyboard focus to the combo box whenever the console is
        // turned on
        setEditState(false);

        if (windowState == CHAT_DISABLED)
            windowState = CHAT_ENABLED;
    }

    void GUIChat::onClose()
    {
        setEditState(false);
    }

    bool GUIChat::exit()
    {
        //WindowBase::exit();
        return true;
    }

    bool GUIChat::getEditState()
    {
        return editState;
    }

    void GUIChat::acceptCommand(MyGUI::EditBox *_sender)
    {
        const std::string &cm = mCommandLine->getOnlyText();

        // If they enter nothing, then it should be canceled.
        // Otherwise, there's no way of closing without having text.
        if (cm.empty())
        {
            mCommandLine->setCaption("");
            setEditState(false);
            return;
        }

        LOG_MESSAGE_SIMPLE(TimedLog::LOG_INFO, "Player: %s", cm.c_str());

        // Add the command to the history, and set the current pointer to
        // the end of the list
        if (mCommandHistory.empty() || mCommandHistory.back() != cm)
            mCommandHistory.push_back(cm);
        mCurrent = mCommandHistory.end();
        mEditString.clear();

        // Reset the command line before the command execution.
        // It prevents the re-triggering of the acceptCommand() event for the same command
        // during the actual command execution
        mCommandLine->setCaption("");
        setEditState(false);
        send(cm);
    }

    void GUIChat::onResChange(int width, int height)
    {
        setCoord(10,10, width-10, height/2);
    }

    void GUIChat::setFont(const std::string &fntName)
    {
        mHistory->setFontName(fntName);
        mCommandLine->setFontName(fntName);
        mSuggestionList->setFontName(fntName);
    }

    void GUIChat::print(const std::string &msg, const std::string &color)
    {
        if (windowState == CHAT_HIDDENMODE && !isVisible())
        {
            setVisible(true);
        }

        if(msg.size() == 0)
        {
            clean();
            LOG_MESSAGE_SIMPLE(TimedLog::LOG_INFO, "Chat cleaned");
        }
        else
        {
            mHistory->addText(color + msg);
            LOG_MESSAGE_SIMPLE(TimedLog::LOG_INFO, "%s", msg.c_str());
        }
    }

    void GUIChat::printOK(const std::string &msg)
    {
        print(msg + "\n", "#FF00FF");
    }

    void GUIChat::printError(const std::string &msg)
    {
        print(msg + "\n", "#FF2222");
    }

    void GUIChat::send(const std::string &str)
    {
        LocalPlayer *localPlayer = Main::get().getLocalPlayer();

        Networking *networking = Main::get().getNetworking();

        localPlayer->chatMessage = str;

        networking->getPlayerPacket(ID_CHAT_MESSAGE)->setPlayer(localPlayer);
        networking->getPlayerPacket(ID_CHAT_MESSAGE)->Send();
    }

    void GUIChat::clean()
    {
        mHistory->setCaption("");
    }

    void GUIChat::pressedChatMode()
    {
        windowState++;
        if (windowState == 3) windowState = 0;

        std::string chatMode = windowState == CHAT_DISABLED ? "Chat hidden" :
                               windowState == CHAT_ENABLED ? "Chat visible" :
                               "Chat appearing when needed";

        LOG_MESSAGE_SIMPLE(TimedLog::LOG_VERBOSE, "Switch chat mode to %s", chatMode.c_str());
        MWBase::Environment::get().getWindowManager()->messageBox(chatMode);

        switch (windowState)
        {
            case CHAT_DISABLED:
                setVisible(false);
                setEditState(false);
                break;
            case CHAT_ENABLED:
                setVisible(true);
                break;
            default: //CHAT_HIDDENMODE
                setVisible(true);
                curTime = 0;
        }
    }

    void GUIChat::setEditState(bool state)
    {
        editState = state;
        mCommandLine->setVisible(editState);

        if (!editState)
        {
            mSuggestionsActive = false;
            hideSuggestions();
        }
        MWBase::Environment::get().getWindowManager()->setKeyFocusWidget(editState ? mCommandLine : nullptr);
    }

    void GUIChat::pressedSay()
    {
        if (windowState == CHAT_DISABLED)
            return;

        if (!mCommandLine->getVisible())
            LOG_MESSAGE_SIMPLE(TimedLog::LOG_VERBOSE, "Opening chat.");

        if (windowState == CHAT_HIDDENMODE)
        {
            setVisible(true);
            curTime = 0;
        }

        setEditState(true);
    }

    void GUIChat::keyPress(MyGUI::Widget *_sender, MyGUI::KeyCode key, MyGUI::Char _char)
    {
        bool suggesting = mSuggestionBox->getVisible() && !mMatches.empty();

        // Tab fills in the selected command, and further Tabs cycle through the suggestions
        if (key == MyGUI::KeyCode::Tab)
        {
            if (suggesting && !mArgumentMode)
                completeSuggestion(MyGUI::InputManager::getInstance().isShiftPressed());
            return;
        }

        // While command names are being suggested, the arrows pick one instead of browsing history
        if (suggesting && !mArgumentMode && (key == MyGUI::KeyCode::ArrowUp || key == MyGUI::KeyCode::ArrowDown))
        {
            int count = static_cast<int>(mMatches.size());
            mSelected = (mSelected + (key == MyGUI::KeyCode::ArrowUp ? count - 1 : 1)) % count;
            showSuggestions();
            return;
        }

        if (mCommandHistory.empty()) return;

        // Traverse history with up and down arrows
        if (key == MyGUI::KeyCode::ArrowUp)
        {
            // If the user was editing a string, store it for later
            if (mCurrent == mCommandHistory.end())
                mEditString = mCommandLine->getOnlyText();

            if (mCurrent != mCommandHistory.begin())
            {
                --mCurrent;
                mCommandLine->setCaption(*mCurrent);
                mSuggestionsActive = false;
                hideSuggestions();
            }
        }
        else if (key == MyGUI::KeyCode::ArrowDown)
        {
            if (mCurrent != mCommandHistory.end())
            {
                ++mCurrent;

                if (mCurrent != mCommandHistory.end())
                    mCommandLine->setCaption(*mCurrent);
                else
                    // Restore the edit string
                    mCommandLine->setCaption(mEditString);

                mSuggestionsActive = false;
                hideSuggestions();
            }
        }

    }

    void GUIChat::textChanged(MyGUI::EditBox *_sender)
    {
        mSuggestionsActive = true;
        mCycling = false;
        updateSuggestions();
    }

    void GUIChat::setCommands(const std::vector<ChatCommand> &commands)
    {
        mCommands = commands;
        mMatches.clear();
        mCycling = false;

        if (editState && mSuggestionsActive)
            updateSuggestions();
        else
            hideSuggestions();
    }

    static std::string toLower(std::string str)
    {
        std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
        return str;
    }

    // MyGUI treats # as the start of a colour code, so a literal one has to be doubled
    static std::string escapeColourCodes(const std::string &str)
    {
        std::string result;
        result.reserve(str.size());

        for (char c : str)
        {
            result += c;
            if (c == '#')
                result += '#';
        }

        return result;
    }

    bool GUIChat::parseCommandLine(std::string &name, bool &hasArguments)
    {
        const std::string &text = mCommandLine->getOnlyText();

        if (text.empty() || text[0] != '/')
            return false;

        size_t space = text.find(' ');
        hasArguments = space != std::string::npos;
        name = toLower(text.substr(1, hasArguments ? space - 1 : std::string::npos));
        return true;
    }

    void GUIChat::updateSuggestions()
    {
        mMatches.clear();
        mSelected = 0;

        std::string name;
        bool hasArguments;

        if (!mSuggestionsActive || mCommands.empty() || !parseCommandLine(name, hasArguments))
        {
            hideSuggestions();
            return;
        }

        mArgumentMode = hasArguments;

        if (hasArguments)
        {
            // Once the command name is finished, show how that command is used
            for (size_t i = 0; i < mCommands.size(); ++i)
            {
                if (toLower(mCommands[i].name) == name)
                    mMatches.push_back(i);
            }
        }
        else
        {
            // Commands starting with what was typed come first, then ones containing it,
            // each shown once even when a command has several usages
            std::vector<size_t> prefixMatches, otherMatches;
            std::vector<std::string> seen;

            for (size_t i = 0; i < mCommands.size(); ++i)
            {
                std::string commandName = toLower(mCommands[i].name);

                if (std::find(seen.begin(), seen.end(), commandName) != seen.end())
                    continue;

                size_t position = commandName.find(name);

                if (position == 0)
                    prefixMatches.push_back(i);
                else if (position != std::string::npos)
                    otherMatches.push_back(i);
                else
                    continue;

                seen.push_back(commandName);
            }

            auto byName = [this](size_t a, size_t b) { return toLower(mCommands[a].name) < toLower(mCommands[b].name); };
            std::sort(prefixMatches.begin(), prefixMatches.end(), byName);
            std::sort(otherMatches.begin(), otherMatches.end(), byName);

            mMatches = prefixMatches;
            mMatches.insert(mMatches.end(), otherMatches.begin(), otherMatches.end());
        }

        if (mMatches.empty())
            hideSuggestions();
        else
            showSuggestions();
    }

    void GUIChat::showSuggestions()
    {
        static const int maxLines = 8;
        static const size_t maxDescriptionLength = 70;

        int count = static_cast<int>(mMatches.size());
        int first = 0;

        // Scroll the list so the selected command stays visible
        if (count > maxLines)
            first = std::clamp(mSelected - maxLines / 2, 0, count - maxLines);

        int last = std::min(count, first + maxLines);

        std::string text;

        if (first > 0)
            text += "#808080  ...\n";

        for (int i = first; i < last; ++i)
        {
            const ChatCommand &command = mCommands[mMatches[i]];
            bool selected = !mArgumentMode && i == mSelected;

            std::string usage = command.usage.empty() ? "/" + command.name : command.usage;
            std::string description = command.description;

            if (description.size() > maxDescriptionLength)
                description = description.substr(0, maxDescriptionLength - 3) + "...";

            text += selected ? "#FFD700> " : "#FFFFFF  ";
            text += escapeColourCodes(usage);

            if (!description.empty())
                text += "#A0A0A0  " + escapeColourCodes(description);

            if (i + 1 < last || last < count)
                text += "\n";
        }

        if (last < count)
            text += "#808080  ... " + std::to_string(count - last) + " more";

        mSuggestionList->setCaption(text);

        // Fit the box to its lines and sit it right above the command line
        int height = mSuggestionList->getTextSize().height + 10;
        int width = mCommandLine->getWidth();
        mSuggestionBox->setCoord(mCommandLine->getLeft(), mCommandLine->getTop() - height, width, height);
        mSuggestionList->setCoord(6, 4, width - 12, height - 8);
        mSuggestionBox->setVisible(true);
    }

    void GUIChat::hideSuggestions()
    {
        mSuggestionBox->setVisible(false);
    }

    void GUIChat::completeSuggestion(bool backwards)
    {
        int count = static_cast<int>(mMatches.size());

        if (mCycling)
            mSelected = (mSelected + (backwards ? count - 1 : 1)) % count;

        mCycling = true;

        // setCaption doesn't fire eventEditTextChange, so the suggestion list stays as it is
        std::string completed = "/" + mCommands[mMatches[mSelected]].name;

        // With one possible command, finish it and move on to its arguments
        if (count == 1)
        {
            completed += " ";
            mCycling = false;
        }

        mCommandLine->setCaption(completed);
        mCommandLine->setTextCursor(completed.size());

        if (count == 1)
            updateSuggestions();
        else
            showSuggestions();
    }

    void GUIChat::update(float dt)
    {
        if (windowState == CHAT_HIDDENMODE && !editState && isVisible())
        {
            curTime += dt;
            if (curTime >= delay)
            {
                setEditState(false);
                setVisible(false);
            }
        }
    }

    void GUIChat::setDelay(float newDelay)
    {
        this->delay = newDelay;
    }
}
