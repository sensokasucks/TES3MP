#ifndef OPENMW_MECHANICS_SUMMONING_H
#define OPENMW_MECHANICS_SUMMONING_H

#include <string_view>
#include <utility>
namespace ESM
{
    class RefId;
}
namespace MWWorld
{
    class Ptr;
}

namespace MWMechanics
{
    bool isSummoningEffect(int effectId);

    ESM::RefId getSummonedCreature(int effectId);

    void purgeSummonEffect(const MWWorld::Ptr& summoner, const std::pair<int, int>& summon);

    /*
        Start of tes3mp change (major)

        Take the sourceId and duration of the summoning effect so they can be sent to the server
    */
    int summonCreature(int effectId, const MWWorld::Ptr& summoner, const std::string& sourceId, float duration);
    /*
        End of tes3mp change (major)
    */

    void updateSummons(const MWWorld::Ptr& summoner, bool cleanup);
}

#endif
