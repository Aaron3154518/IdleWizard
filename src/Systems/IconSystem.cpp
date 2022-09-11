#include "IconSystem.h"

namespace IconSystem {
RenderTextureCPtr AnimationSet::operator[](const AnimationData& data) {
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
        rData = std::make_shared<RenderTexture>(data);
    }
    return rData;
}

RenderTextureCPtr Get(const AnimationData& data) {
    static std::unordered_map<int, AnimationSet> ANIMS;

    return ANIMS[data.frame_ms][data];
}

RenderTextureCPtr Get(const std::string& file) {
    static std::unordered_map<std::string, RenderTexturePtr> IMGS;

    auto& rData = IMGS[file];
    if (!rData) {
        rData = std::make_shared<RenderTexture>(file);
    }
    return rData;
}
}  // namespace IconSystem
