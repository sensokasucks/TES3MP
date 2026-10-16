#ifndef OPENMW_HOOKSAPI_HPP
#define OPENMW_HOOKSAPI_HPP

#include "../Types.hpp"

class Player;

#define HOOKSAPI \
    {"GetAttackTargetIsPlayer",    SCRIPT_FUNCTION(HookFunctions::GetAttackTargetIsPlayer)},\
    {"GetAttackTargetPid",         SCRIPT_FUNCTION(HookFunctions::GetAttackTargetPid)},\
    {"GetAttackTargetRefId",       SCRIPT_FUNCTION(HookFunctions::GetAttackTargetRefId)},\
    {"GetAttackTargetRefNum",      SCRIPT_FUNCTION(HookFunctions::GetAttackTargetRefNum)},\
    {"GetAttackTargetMpNum",       SCRIPT_FUNCTION(HookFunctions::GetAttackTargetMpNum)},\
    {"GetAttackType",              SCRIPT_FUNCTION(HookFunctions::GetAttackType)},\
    {"GetAttackSuccess",           SCRIPT_FUNCTION(HookFunctions::GetAttackSuccess)},\
    {"GetAttackDamage",            SCRIPT_FUNCTION(HookFunctions::GetAttackDamage)},\
    {"GetAttackBlocked",           SCRIPT_FUNCTION(HookFunctions::GetAttackBlocked)},\
    {"GetAttackKnockdown",         SCRIPT_FUNCTION(HookFunctions::GetAttackKnockdown)},\
    {"GetAttackStrength",          SCRIPT_FUNCTION(HookFunctions::GetAttackStrength)},\
    {"GetAttackRangedWeaponId",    SCRIPT_FUNCTION(HookFunctions::GetAttackRangedWeaponId)},\
    {"GetAttackRangedAmmoId",      SCRIPT_FUNCTION(HookFunctions::GetAttackRangedAmmoId)},\
    \
    {"GetCastTargetIsPlayer",      SCRIPT_FUNCTION(HookFunctions::GetCastTargetIsPlayer)},\
    {"GetCastTargetPid",           SCRIPT_FUNCTION(HookFunctions::GetCastTargetPid)},\
    {"GetCastTargetRefId",         SCRIPT_FUNCTION(HookFunctions::GetCastTargetRefId)},\
    {"GetCastTargetRefNum",        SCRIPT_FUNCTION(HookFunctions::GetCastTargetRefNum)},\
    {"GetCastTargetMpNum",         SCRIPT_FUNCTION(HookFunctions::GetCastTargetMpNum)},\
    {"GetCastType",                SCRIPT_FUNCTION(HookFunctions::GetCastType)},\
    {"GetCastSuccess",             SCRIPT_FUNCTION(HookFunctions::GetCastSuccess)},\
    {"GetCastSpellId",             SCRIPT_FUNCTION(HookFunctions::GetCastSpellId)},\
    {"GetCastItemId",              SCRIPT_FUNCTION(HookFunctions::GetCastItemId)},\
    \
    {"AddHealthThreshold",         SCRIPT_FUNCTION(HookFunctions::AddHealthThreshold)},\
    {"ClearHealthThresholds",      SCRIPT_FUNCTION(HookFunctions::ClearHealthThresholds)},\
    \
    {"GetPlayerMenu",              SCRIPT_FUNCTION(HookFunctions::GetPlayerMenu)},\
    {"IsPlayerInMenu",             SCRIPT_FUNCTION(HookFunctions::IsPlayerInMenu)},\
    {"GetPlayerIdleTime",          SCRIPT_FUNCTION(HookFunctions::GetPlayerIdleTime)}

class HookFunctions
{
public:

    /**
    * \brief Check whether the target of the player's last attack was a player.
    *
    * The attack getters read the attack that triggered the latest OnPlayerAttack(pid) event.
    *
    * \param pid The player ID.
    * \return Whether the target is a player.
    */
    static bool GetAttackTargetIsPlayer(unsigned short pid) noexcept;

    /**
    * \brief Get the player ID of the target of the player's last attack.
    *
    * \param pid The player ID.
    * \return The target's player ID, or -1 if the target is not a player.
    */
    static int GetAttackTargetPid(unsigned short pid) noexcept;

    /**
    * \brief Get the refId of the target of the player's last attack, if it was not a player.
    *
    * \param pid The player ID.
    * \return The refId, or an empty string if there was no actor target.
    */
    static const char *GetAttackTargetRefId(unsigned short pid) noexcept;

    /**
    * \brief Get the refNum of the target of the player's last attack.
    *
    * \param pid The player ID.
    * \return The refNum.
    */
    static unsigned int GetAttackTargetRefNum(unsigned short pid) noexcept;

    /**
    * \brief Get the mpNum of the target of the player's last attack.
    *
    * \param pid The player ID.
    * \return The mpNum.
    */
    static unsigned int GetAttackTargetMpNum(unsigned short pid) noexcept;

    /**
    * \brief Get the type of the player's last attack.
    *
    * \param pid The player ID.
    * \return 0 for melee, 1 for ranged.
    */
    static unsigned int GetAttackType(unsigned short pid) noexcept;

    /**
    * \brief Check whether the player's last attack connected.
    *
    * \param pid The player ID.
    * \return Whether the attack succeeded.
    */
    static bool GetAttackSuccess(unsigned short pid) noexcept;

    /**
    * \brief Get the damage of the player's last attack.
    *
    * \param pid The player ID.
    * \return The damage.
    */
    static double GetAttackDamage(unsigned short pid) noexcept;

    /**
    * \brief Check whether the player's last attack was blocked.
    *
    * \param pid The player ID.
    * \return Whether it was blocked.
    */
    static bool GetAttackBlocked(unsigned short pid) noexcept;

    /**
    * \brief Check whether the player's last attack knocked its target down.
    *
    * \param pid The player ID.
    * \return Whether it caused a knockdown.
    */
    static bool GetAttackKnockdown(unsigned short pid) noexcept;

    /**
    * \brief Get the strength of the player's last ranged attack.
    *
    * \param pid The player ID.
    * \return The attack strength (0 to 1).
    */
    static double GetAttackStrength(unsigned short pid) noexcept;

    /**
    * \brief Get the refId of the weapon used for the player's last ranged attack.
    *
    * \param pid The player ID.
    * \return The refId.
    */
    static const char *GetAttackRangedWeaponId(unsigned short pid) noexcept;

    /**
    * \brief Get the refId of the ammo used for the player's last ranged attack.
    *
    * \param pid The player ID.
    * \return The refId.
    */
    static const char *GetAttackRangedAmmoId(unsigned short pid) noexcept;

    /**
    * \brief Check whether the target of the player's last spell cast was a player.
    *
    * The cast getters read the cast that triggered the latest OnPlayerCast(pid) event.
    *
    * \param pid The player ID.
    * \return Whether the target is a player.
    */
    static bool GetCastTargetIsPlayer(unsigned short pid) noexcept;

    /**
    * \brief Get the player ID of the target of the player's last cast.
    *
    * \param pid The player ID.
    * \return The target's player ID, or -1 if the target is not a player.
    */
    static int GetCastTargetPid(unsigned short pid) noexcept;

    /**
    * \brief Get the refId of the target of the player's last cast, if it was not a player.
    *
    * \param pid The player ID.
    * \return The refId, or an empty string if there was no actor target.
    */
    static const char *GetCastTargetRefId(unsigned short pid) noexcept;

    /**
    * \brief Get the refNum of the target of the player's last cast.
    *
    * \param pid The player ID.
    * \return The refNum.
    */
    static unsigned int GetCastTargetRefNum(unsigned short pid) noexcept;

    /**
    * \brief Get the mpNum of the target of the player's last cast.
    *
    * \param pid The player ID.
    * \return The mpNum.
    */
    static unsigned int GetCastTargetMpNum(unsigned short pid) noexcept;

    /**
    * \brief Get the type of the player's last cast.
    *
    * \param pid The player ID.
    * \return 0 for a spell, 1 for an enchanted item.
    */
    static unsigned int GetCastType(unsigned short pid) noexcept;

    /**
    * \brief Check whether the player's last spell cast succeeded.
    *
    * \param pid The player ID.
    * \return Whether the cast succeeded. Always true for item casts.
    */
    static bool GetCastSuccess(unsigned short pid) noexcept;

    /**
    * \brief Get the id of the spell in the player's last cast.
    *
    * \param pid The player ID.
    * \return The spell id, or an empty string for item casts.
    */
    static const char *GetCastSpellId(unsigned short pid) noexcept;

    /**
    * \brief Get the refId of the item used in the player's last cast.
    *
    * \param pid The player ID.
    * \return The item refId, or an empty string for spell casts.
    */
    static const char *GetCastItemId(unsigned short pid) noexcept;

    /**
    * \brief Add a health fraction that fires OnPlayerHealthThreshold when a player's health crosses it.
    *
    * With no thresholds added, OnPlayerHealthThreshold never fires.
    *
    * \param fraction The fraction of base health, between 0 and 1 (e.g. 0.25).
    * \return void
    */
    static void AddHealthThreshold(double fraction) noexcept;

    /**
    * \brief Remove every health threshold.
    *
    * \return void
    */
    static void ClearHealthThresholds() noexcept;

    /**
    * \brief Get the name of the menu the player has open.
    *
    * \param pid The player ID.
    * \return The menu name (e.g. "inventory", "dialogue", "container"), or an empty string when no menu is open.
    */
    static const char *GetPlayerMenu(unsigned short pid) noexcept;

    /**
    * \brief Check whether the player has any menu open.
    *
    * \param pid The player ID.
    * \return Whether a menu is open.
    */
    static bool IsPlayerInMenu(unsigned short pid) noexcept;

    /**
    * \brief Get how long the player has been idle.
    *
    * Movement, attacks, casts, menu changes and chat messages count as activity.
    *
    * \param pid The player ID.
    * \return Seconds since the player's last activity.
    */
    static double GetPlayerIdleTime(unsigned short pid) noexcept;
};

namespace Hooks
{
    // Called from packet processors
    void markActive(Player &player);
    void checkHealthThresholds(Player &player);
}

#endif //OPENMW_HOOKSAPI_HPP
