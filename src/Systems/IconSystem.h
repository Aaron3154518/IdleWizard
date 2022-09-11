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

RenderTextureCPtr Get(const AnimationData& data);
RenderTextureCPtr Get(const std::string& file);
}  // namespace IconSystem

#endif
