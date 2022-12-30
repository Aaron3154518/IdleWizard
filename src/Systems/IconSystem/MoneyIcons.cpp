#include "MoneyIcons.h"

namespace MoneyIcons {
RenderTextureCPtr GetMoneyIcon(const ParameterSystem::ValueParam& param) {
    static const AnimationData CRYSTAL_MAGIC{"res/money/crystal_magic.png", 6,
                                             150},
        CRYSTAL_SHARDS{"res/money/crystal_shard.png", 10, 100},
        CATALYST_MAGIC{"res/money/catalyst_magic.png", 8, 150};

    AnimationData data;
    switch (param.id()) {
        case CRYSTAL:
            switch (param.key()) {
                case Crystal::Param::Magic:
                    data = CRYSTAL_MAGIC;
                    break;
                case Crystal::Param::Shards:
                    data = CRYSTAL_SHARDS;
                    break;
            }
            break;
        case CATALYST:
            switch (param.key()) {
                case Catalyst::Param::Magic:
                    data = CATALYST_MAGIC;
                    break;
                case Catalyst::Param::FBRegCnt:
                    return IconSystem::Get(Wizard::Constants::FB_IMG());
                case Catalyst::Param::FBPowCnt:
                    return IconSystem::Get(PowerWizard::Constants::FB_IMG());
                case Catalyst::Param::FBPoiCnt:
                    return IconSystem::Get(PoisonWizard::Constants::GLOB_IMG());
            };
            break;
    };

    return IconSystem::Get(data);
}
}  // namespace MoneyIcons
