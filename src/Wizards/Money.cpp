#include "Money.h"

namespace Money {
RenderTextureCPtr GetMoneyIcon(const ParameterSystem::ValueParam& param) {
    static const AnimationData CRYSTAL_MAGIC{"res/money/crystal_magic.png", 6,
                                             150},
        CRYSTAL_SHARDS{"res/money/crystal_shard.png", 10, 100},
        CATALYST_MAGIC{"res/money/catalyst_magic.png", 8, 150};

    AnimationData data;
    switch (param.mId) {
        case CRYSTAL:
            switch (param.mKey) {
                case CrystalParams::Magic:
                    data = CRYSTAL_MAGIC;
                    break;
                case CrystalParams::Shards:
                    data = CRYSTAL_SHARDS;
                    break;
            }
            break;
        case CATALYST:
            switch (param.mKey) {
                case CatalystParams::Magic:
                    data = CATALYST_MAGIC;
                    break;
                case CatalystParams::FBRegCnt:
                    return IconSystem::Get(WizardDefs::FB_IMG());
                case CatalystParams::FBPowCnt:
                    return IconSystem::Get(PowerWizardDefs::FB_IMG());
                case CatalystParams::FBPoiCnt:
                    return IconSystem::Get(PoisonWizardDefs::GLOB_IMG());
            };
            break;
    };

    return IconSystem::Get(data);
}
}  // namespace Money
