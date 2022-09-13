#include "IconSystem.h"

namespace IconSystem {
// AnimationSet
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

// AnimationManager
RenderTextureCPtr AnimationManager::operator[](const AnimationData& data) {
    return mAnims[data.frame_ms][data];
}
RenderTextureCPtr AnimationManager::operator[](const std::string& file) {
    auto& rData = mImgs[file];
    if (!rData) {
        rData = std::make_shared<RenderTexture>(file);
    }
    return rData;
}

AnimationManager& Get() {
    static AnimationManager ANIM_MANAGER;

    return ANIM_MANAGER;
}

RenderTextureCPtr Get(const AnimationData& data) { return Get()[data]; }

RenderTextureCPtr Get(const std::string& file) { return Get()[file]; }
}  // namespace IconSystem
