#include "IconSystem.h"

namespace IconSystem {
RenderDataCWPtr AnimationSet::operator[](const AnimationData& data) {
    if (!mTimerSub) {
        mTimerSub =
            ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
                [this](Timer& t) {
                    for (auto pair : mAnims) {
                        pair.second->nextFrame();
                    }
                    return true;
                },
                Timer(data.frame_ms));
    }

    auto& rData = mAnims[data.file];
    if (!rData) {
        rData = std::make_shared<RenderData>();
        rData->set(data);
    }
    return rData;
}

RenderDataCWPtr Get(const AnimationData& data) {
    static std::unordered_map<int, AnimationSet> ANIMS;

    return ANIMS[data.frame_ms][data];
}

RenderDataCWPtr Get(const std::string& file) {
    static std::unordered_map<std::string, RenderDataPtr> IMGS;

    auto& rData = IMGS[file];
    if (!rData) {
        rData = std::make_shared<RenderData>();
        rData->set(file);
    }
    return rData;
}
}  // namespace IconSystem
