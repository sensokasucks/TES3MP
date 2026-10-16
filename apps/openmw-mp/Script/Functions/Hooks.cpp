#include "Hooks.hpp"

#include <apps/openmw-mp/Script/ScriptFunctions.hpp>
#include <apps/openmw-mp/Script/Script.hpp>

#include <algorithm>
#include <vector>

static std::vector<double> healthThresholds;

static int getTargetPid(const mwmp::Target &target)
{
    if (!target.isPlayer)
        return -1;

    Player *targetPlayer = Players::getPlayer(target.guid);
    return targetPlayer != nullptr ? targetPlayer->getId() : -1;
}

bool HookFunctions::GetAttackTargetIsPlayer(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, false);

    return player->attack.target.isPlayer;
}

int HookFunctions::GetAttackTargetPid(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, -1);

    return getTargetPid(player->attack.target);
}

const char *HookFunctions::GetAttackTargetRefId(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, "");

    if (player->attack.target.isPlayer)
        return "";

    return player->attack.target.refId.c_str();
}

unsigned int HookFunctions::GetAttackTargetRefNum(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, 0);

    return player->attack.target.refNum;
}

unsigned int HookFunctions::GetAttackTargetMpNum(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, 0);

    return player->attack.target.mpNum;
}

unsigned int HookFunctions::GetAttackType(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, 0);

    return player->attack.type;
}

bool HookFunctions::GetAttackSuccess(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, false);

    return player->attack.success;
}

double HookFunctions::GetAttackDamage(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, 0);

    return player->attack.damage;
}

bool HookFunctions::GetAttackBlocked(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, false);

    return player->attack.block;
}

bool HookFunctions::GetAttackKnockdown(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, false);

    return player->attack.knockdown;
}

double HookFunctions::GetAttackStrength(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, 0);

    return player->attack.attackStrength;
}

const char *HookFunctions::GetAttackRangedWeaponId(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, "");

    return player->attack.rangedWeaponId.c_str();
}

const char *HookFunctions::GetAttackRangedAmmoId(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, "");

    return player->attack.rangedAmmoId.c_str();
}

bool HookFunctions::GetCastTargetIsPlayer(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, false);

    return player->cast.target.isPlayer;
}

int HookFunctions::GetCastTargetPid(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, -1);

    return getTargetPid(player->cast.target);
}

const char *HookFunctions::GetCastTargetRefId(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, "");

    if (player->cast.target.isPlayer)
        return "";

    return player->cast.target.refId.c_str();
}

unsigned int HookFunctions::GetCastTargetRefNum(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, 0);

    return player->cast.target.refNum;
}

unsigned int HookFunctions::GetCastTargetMpNum(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, 0);

    return player->cast.target.mpNum;
}

unsigned int HookFunctions::GetCastType(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, 0);

    return player->cast.type;
}

bool HookFunctions::GetCastSuccess(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, false);

    if (player->cast.type == mwmp::Cast::ITEM)
        return true;

    return player->cast.success;
}

const char *HookFunctions::GetCastSpellId(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, "");

    if (player->cast.type == mwmp::Cast::ITEM)
        return "";

    return player->cast.spellId.c_str();
}

const char *HookFunctions::GetCastItemId(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, "");

    if (player->cast.type != mwmp::Cast::ITEM)
        return "";

    return player->cast.itemId.c_str();
}

void HookFunctions::AddHealthThreshold(double fraction) noexcept
{
    if (fraction <= 0 || fraction >= 1)
    {
        LOG_MESSAGE_SIMPLE(TimedLog::LOG_WARN, "AddHealthThreshold: %f is not between 0 and 1", fraction);
        return;
    }

    if (std::find(healthThresholds.begin(), healthThresholds.end(), fraction) == healthThresholds.end())
        healthThresholds.push_back(fraction);
}

void HookFunctions::ClearHealthThresholds() noexcept
{
    healthThresholds.clear();
}

const char *HookFunctions::GetPlayerMenu(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, "");

    return player->menu.c_str();
}

bool HookFunctions::IsPlayerInMenu(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, false);

    return !player->menu.empty();
}

double HookFunctions::GetPlayerIdleTime(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, 0);

    return std::chrono::duration<double>(std::chrono::steady_clock::now() - player->lastActivity).count();
}

void Hooks::markActive(Player &player)
{
    player.lastActivity = std::chrono::steady_clock::now();
}

void Hooks::checkHealthThresholds(Player &player)
{
    const auto &health = player.creatureStats.mDynamic[0];

    if (health.mBase <= 0)
        return;

    double ratio = health.mCurrent / health.mBase;
    double previous = player.lastHealthRatio;
    player.lastHealthRatio = ratio;

    // The first update after joining only records the starting health
    if (previous < 0 || healthThresholds.empty())
        return;

    for (double threshold : healthThresholds)
    {
        if (previous >= threshold && ratio < threshold)
            Script::Call<Script::CallbackIdentity("OnPlayerHealthThreshold")>(player.getId(), threshold, true);
        else if (previous < threshold && ratio >= threshold)
            Script::Call<Script::CallbackIdentity("OnPlayerHealthThreshold")>(player.getId(), threshold, false);
    }
}
