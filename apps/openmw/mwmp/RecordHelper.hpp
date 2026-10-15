#ifndef OPENMW_RECORDHELPER_HPP
#define OPENMW_RECORDHELPER_HPP

#include <components/openmw-mp/Base/BaseWorldstate.hpp>

#include "../mwbase/environment.hpp"

namespace RecordHelper
{
    void overrideRecord(const mwmp::ActivatorRecord& record);
    void overrideRecord(const mwmp::ApparatusRecord& record);
    void overrideRecord(const mwmp::ArmorRecord& record);
    void overrideRecord(const mwmp::BodyPartRecord& record);
    void overrideRecord(const mwmp::BookRecord& record);
    void overrideRecord(const mwmp::CellRecord& record);
    void overrideRecord(const mwmp::ClothingRecord& record);
    void overrideRecord(const mwmp::ContainerRecord& record);
    void overrideRecord(const mwmp::CreatureRecord& record);
    void overrideRecord(const mwmp::DoorRecord& record);
    void overrideRecord(const mwmp::EnchantmentRecord& record);
    void overrideRecord(const mwmp::GameSettingRecord& record);
    void overrideRecord(const mwmp::IngredientRecord& record);
    void overrideRecord(const mwmp::LightRecord& record);
    void overrideRecord(const mwmp::LockpickRecord& record);
    void overrideRecord(const mwmp::MiscellaneousRecord& record);
    void overrideRecord(const mwmp::NpcRecord& record);
    void overrideRecord(const mwmp::PotionRecord& record);
    void overrideRecord(const mwmp::ProbeRecord& record);
    void overrideRecord(const mwmp::RepairRecord& record);
    void overrideRecord(const mwmp::ScriptRecord& record);
    void overrideRecord(const mwmp::SoundRecord& record);
    void overrideRecord(const mwmp::SpellRecord& record);
    void overrideRecord(const mwmp::StaticRecord& record);
    void overrideRecord(const mwmp::WeaponRecord& record);

    template<class RecordType>
    void overrideRecord(const RecordType &record)
    {
        MWBase::World *world = MWBase::Environment::get().getWorld();

        world->getModifiableStore().overrideRecord(record);
    }

    template<class RecordType>
    const RecordType *createRecord(const RecordType &record)
    {
        MWBase::World *world = MWBase::Environment::get().getWorld();

        // ESMStore::insert() gives new records a generated RefId in 0.51, and calling getRefIdString()
        // on one of those throws, which tes3mp code does all over (e.g. when placing a DedicatedPlayer),
        // so give them a unique string id instead, like 0.47's "$dynamic" ids
        static unsigned int dynamicCount = 0;
        RecordType finalRecord = record;

        do
        {
            finalRecord.mId = ESM::RefId::stringRefId("$tes3mp_dynamic" + std::to_string(dynamicCount++));
        }
        while (world->getStore().get<RecordType>().search(finalRecord.mId) != nullptr);

        return world->getModifiableStore().overrideRecord(finalRecord);
    }

    template<class RecordType>
    bool doesRecordIdExist(const ESM::RefId& id)
    {
        MWBase::World *world = MWBase::Environment::get().getWorld();

        return world->getStore().get<RecordType>().search(id) != nullptr;
    }

    template<class RecordType>
    bool doesRecordIdExist(const std::string& id)
    {
        return doesRecordIdExist<RecordType>(ESM::RefId::stringRefId(id));
    }

    void createPlaceholderInteriorCell();
    const std::string getPlaceholderInteriorCellName();

    const std::string placeholderInteriorCellName = "$Transitional Void";
}


#endif //OPENMW_RECORDHELPER_HPP
