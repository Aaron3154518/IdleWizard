#include "Money.h"

namespace Money {
const RenderData& GetMoneyIcon(const ParameterSystem::ValueParam& param) {
    static RenderData DEFAULT,
        CRYSTAL_MAGIC = RenderData().set("res/money/crystal_magic.png", 6),
        CRYSTAL_SHARDS = RenderData().set("res/money/crystal_shard.png"),
        CATALYST_MAGIC = RenderData().set("res/wizards/catalyst.png");
    static TimerObservable::SubscriptionPtr CRYS_MAG_SUB =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            [](Timer& t) {
                CRYSTAL_MAGIC.nextFrame();
                return true;
            },
            Timer(100));

    switch (param.mId) {
        case CRYSTAL:
            switch (param.mKey) {
                case CrystalParams::Magic:
                    return CRYSTAL_MAGIC;
                case CrystalParams::Shards:
                    return CRYSTAL_SHARDS;
            }
            break;
        case CATALYST:
            switch (param.mKey) {
                case CatalystParams::Magic:
                    return CATALYST_MAGIC;
            };
            break;
    };

    return DEFAULT;
}
}  // namespace Money
