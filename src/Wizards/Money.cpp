#include "Money.h"

namespace Money {
RenderDataWPtr GetMoneyIcon(const ParameterSystem::ValueParam& param) {
    static RenderDataPtr DEFAULT, CRYSTAL_MAGIC, CRYSTAL_SHARDS, CATALYST_MAGIC;
    static TimerObservable::SubscriptionPtr CRYS_MAG_SUB, CRYS_SHARD_SUB,
        CAT_MAG_SUB;
    if (!CRYSTAL_MAGIC) {
        auto timerObs = ServiceSystem::Get<TimerService, TimerObservable>();

        CRYSTAL_MAGIC = std::make_shared<RenderData>();
        CRYSTAL_MAGIC->set("res/money/crystal_magic.png", 6);
        CRYS_MAG_SUB = timerObs->subscribe(
            [](Timer& t) {
                CRYSTAL_MAGIC->nextFrame();
                return true;
            },
            Timer(150));

        CRYSTAL_SHARDS = std::make_shared<RenderData>();
        CRYSTAL_SHARDS->set("res/money/crystal_shard.png", 9);
        CRYS_SHARD_SUB = timerObs->subscribe(
            [](Timer& t) {
                CRYSTAL_SHARDS->nextFrame();
                t.length = CRYSTAL_SHARDS->getFrame() == 0 ? 200 : 100;
                return true;
            },
            Timer(100));

        CATALYST_MAGIC = std::make_shared<RenderData>();
        CATALYST_MAGIC->set("res/money/catalyst_magic.png", 8);
        CAT_MAG_SUB = timerObs->subscribe(
            [](Timer& t) {
                CATALYST_MAGIC->nextFrame();
                return true;
            },
            Timer(150));
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
