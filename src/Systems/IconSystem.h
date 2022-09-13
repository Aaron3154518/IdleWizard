#ifndef ICON_SYSTEM_H
#define ICON_SYSTEM_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>

#include <unordered_map>

namespace IconSystem {
class AnimationSet {
   public:
    RenderTextureCPtr operator[](const AnimationData& data);

   private:
    std::unordered_map<std::string, RenderTexturePtr> mAnims;
    TimerObservable::SubscriptionPtr mTimerSub;
};

class AnimationManager {
   public:
    RenderTextureCPtr operator[](const AnimationData& data);
    RenderTextureCPtr operator[](const std::string& file);

   private:
    std::unordered_map<int, AnimationSet> mAnims;
    std::unordered_map<std::string, RenderTexturePtr> mImgs;
};

AnimationManager& Get();
RenderTextureCPtr Get(const AnimationData& data);
RenderTextureCPtr Get(const std::string& file);
}  // namespace IconSystem

#endif
