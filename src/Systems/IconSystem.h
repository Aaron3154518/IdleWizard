#ifndef ICON_SYSTEM_H
#define ICON_SYSTEM_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>

#include <unordered_map>

namespace IconSystem {
class AnimationSet {
   public:
    RenderDataCWPtr operator[](const AnimationData& data);

   private:
    std::unordered_map<std::string, RenderDataPtr> mAnims;
    TimerObservable::SubscriptionPtr mTimerSub;
};

RenderDataCWPtr Get(const AnimationData& data);
RenderDataCWPtr Get(const std::string& file);
}  // namespace IconSystem

#endif
