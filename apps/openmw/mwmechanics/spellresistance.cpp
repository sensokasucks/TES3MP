#include "spellresistance.hpp"

#include <components/misc/rng.hpp>

#include <components/esm3/loadmgef.hpp>
#include <components/esm3/loadspel.hpp>

#include "../mwbase/environment.hpp"
#include "../mwbase/world.hpp"

#include "../mwworld/class.hpp"
#include "../mwworld/esmstore.hpp"
#include "../mwworld/vanilladebug.hpp"

#include "creaturestats.hpp"
#include "spellutil.hpp"

namespace MWMechanics
{

    float getEffectMultiplier(ESM::RefId effectId, const MWWorld::Ptr& actor, const MWWorld::Ptr& caster,
        const ESM::Spell* spell, const MagicEffects* effects)
    {
        if (!actor.getClass().isActor())
            return 1;

        float resistance = getEffectResistance(effectId, actor, caster, spell, effects);
        return 1 - resistance / 100.f;
    }

    float getEffectResistance(ESM::RefId effectId, const MWWorld::Ptr& actor, const MWWorld::Ptr& caster,
        const ESM::Spell* spell, const MagicEffects* effects)
    {
        // Effects with no resistance attribute belonging to them can not be resisted
        if (ESM::MagicEffect::getResistanceEffect(effectId).empty())
            return 0.f;

        const auto magicEffect = MWBase::Environment::get().getESMStore()->get<ESM::MagicEffect>().find(effectId);

        const MWMechanics::CreatureStats& stats = actor.getClass().getCreatureStats(actor);
        const MWMechanics::MagicEffects* magicEffects = &stats.getMagicEffects();
        if (effects)
            magicEffects = effects;

        float resistance = getEffectResistanceAttribute(effectId, magicEffects);

        float willpower = stats.getAttribute(ESM::Attribute::Willpower).getModified();
        float luck = stats.getAttribute(ESM::Attribute::Luck).getModified();
        float x = (willpower + 0.1f * luck) * stats.getFatigueTerm();

        // This makes spells that are easy to cast harder to resist and vice versa
        float castChance = 100.f;
        if (spell != nullptr && !caster.isEmpty() && caster.getClass().isActor())
            castChance = getSpellSuccessChance(spell, caster, nullptr, false, false); // Uncapped casting chance
        if (castChance > 0)
            x *= 50 / castChance;

        auto& prng = MWBase::Environment::get().getWorld()->getPrng();
        float roll = Misc::Rng::rollClosedProbability(prng) * 100;
        if (magicEffect->mData.mFlags & ESM::MagicEffect::NoMagnitude)
            roll -= resistance;

        if (x <= roll)
            x = 0;
        else
        {
            if (magicEffect->mData.mFlags & ESM::MagicEffect::NoMagnitude)
                x = 100;
            else
                x = roll / std::min(x, 100.f);
        }

        x = std::min(x + resistance, 100.f);

        /*
            Start of tes3mp addition

            ToggleMagicStats (TMS), with the vanilla wording
        */
        if (MWWorld::VanillaDebug::isEnabled(MWWorld::VanillaDebug::Flag_MagicStats))
        {
            MWWorld::VanillaDebug::print(MWWorld::VanillaDebug::format("Resist check: effect %s, spell %s",
                effectId.toString().c_str(), spell != nullptr ? spell->mId.toString().c_str() : "none"));
            MWWorld::VanillaDebug::print(MWWorld::VanillaDebug::format("            : Caster %s; Target %s",
                MWWorld::VanillaDebug::describe(caster).c_str(), MWWorld::VanillaDebug::describe(actor).c_str()));
            MWWorld::VanillaDebug::print(MWWorld::VanillaDebug::format(
                "           : Target Willpower: %f Luck: %f FatigueMult: %f EffectResist: %d", willpower, luck,
                stats.getFatigueTerm(), static_cast<int>(resistance)));
            MWWorld::VanillaDebug::print(
                MWWorld::VanillaDebug::format("            : roll: %f resisted: %f%%", roll, x));
        }
        /*
            End of tes3mp addition
        */

        return x;
    }

    float getEffectResistanceAttribute(ESM::RefId effectId, const MagicEffects* actorEffects)
    {
        float resistance = 0;

        ESM::RefId resistanceEffect = ESM::MagicEffect::getResistanceEffect(effectId);
        ESM::RefId weaknessEffect = ESM::MagicEffect::getWeaknessEffect(effectId);

        if (!resistanceEffect.empty())
            resistance += actorEffects->getOrDefault(resistanceEffect).getMagnitude();
        if (!weaknessEffect.empty())
            resistance -= actorEffects->getOrDefault(weaknessEffect).getMagnitude();

        if (effectId == ESM::MagicEffect::FireDamage)
            resistance += actorEffects->getOrDefault(ESM::MagicEffect::FireShield).getMagnitude();
        if (effectId == ESM::MagicEffect::ShockDamage)
            resistance += actorEffects->getOrDefault(ESM::MagicEffect::LightningShield).getMagnitude();
        if (effectId == ESM::MagicEffect::FrostDamage)
            resistance += actorEffects->getOrDefault(ESM::MagicEffect::FrostShield).getMagnitude();

        return resistance;
    }
}
