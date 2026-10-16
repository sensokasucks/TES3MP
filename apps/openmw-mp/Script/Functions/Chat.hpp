#ifndef OPENMW_CHATAPI_HPP
#define OPENMW_CHATAPI_HPP

#include "../Types.hpp"

#define CHATAPI \
    {"SendMessage",       SCRIPT_FUNCTION(ChatFunctions::SendMessage)},\
    {"CleanChatForPid",   SCRIPT_FUNCTION(ChatFunctions::CleanChatForPid)},\
    {"CleanChat",         SCRIPT_FUNCTION(ChatFunctions::CleanChat)},\
    \
    {"ClearChatCommands", SCRIPT_FUNCTION(ChatFunctions::ClearChatCommands)},\
    {"AddChatCommand",    SCRIPT_FUNCTION(ChatFunctions::AddChatCommand)},\
    {"SendChatCommands",  SCRIPT_FUNCTION(ChatFunctions::SendChatCommands)}

class ChatFunctions
{
public:

    /**
    * \brief Send a message to a certain player.
    *
    * \param pid The player ID.
    * \param message The contents of the message.
    * \param sendToOtherPlayers Whether this packet should be sent to players other than the
    *                           player attached to the packet (false by default).
    * \param skipAttachedPlayer Whether the packet should skip being sent to the player attached
    *                           to the packet (false by default).
    * \return void
    */
    static void SendMessage(unsigned short pid, const char *message, bool sendToOtherPlayers, bool skipAttachedPlayer) noexcept;

    /**
    * \brief Remove all messages from chat for a certain player.
    *
    * \param pid The player ID.
    * \return void
    */
    static void CleanChatForPid(unsigned short pid);

    /**
    * \brief Remove all messages from chat for everyone on the server.
    *
    * \return void
    */
    static void CleanChat();

    /**
    * \brief Clear the list of chat commands stored for a player, so a new one can be built
    *        with AddChatCommand().
    *
    * \param pid The player ID.
    * \return void
    */
    static void ClearChatCommands(unsigned short pid) noexcept;

    /**
    * \brief Add a command to the list of chat commands stored for a player. The client uses
    *        this list to suggest and autocomplete commands as the player types in chat.
    *
    * \param pid The player ID.
    * \param name The command name without the leading slash, such as "msg".
    * \param usage How the command is used, such as "/msg <pid> <text>".
    * \param description A short explanation of what the command does.
    * \return void
    */
    static void AddChatCommand(unsigned short pid, const char *name, const char *usage, const char *description) noexcept;

    /**
    * \brief Send the list of chat commands stored for a player to that player, replacing
    *        the list their client had before.
    *
    * \param pid The player ID.
    * \return void
    */
    static void SendChatCommands(unsigned short pid) noexcept;
};

#endif //OPENMW_CHATAPI_HPP
