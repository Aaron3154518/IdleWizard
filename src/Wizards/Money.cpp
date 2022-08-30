#include "Money.h"

namespace Money {
const RenderDataPtr& GetMoneyIcon(const ParameterSystem::ValueParam& param) {
    static RenderDataPtr DEFAULT, CRYSTAL_MAGIC, CRYSTAL_SHARDS, CATALYST_MAGIC;
    static TimerObservable::SubscriptionPtr CRYS_MAG_SUB;
    if (!CRYSTAL_MAGIC) {
        CRYSTAL_MAGIC = std::make_shared<RenderData>();
        CRYSTAL_MAGIC->set("res/money/crystal_magic.png", 6);
        CRYS_MAG_SUB =
            ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
                [](Timer& t) {
                    CRYSTAL_MAGIC->nextFrame();
                    return true;
                },
                Timer(150));

        CRYSTAL_SHARDS = std::make_shared<RenderData>();
        CRYSTAL_SHARDS->set("res/money/crystal_shard.png");

        CATALYST_MAGIC = std::make_shared<RenderData>();
        CATALYST_MAGIC->set("res/wizards/catalyst.png");
    }

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
