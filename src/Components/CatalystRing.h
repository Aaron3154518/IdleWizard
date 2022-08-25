#ifndef CATALYST_RING_H
#define CATALYST_RING_H

#include <RenderSystem/Shapes.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/TimeSystem.h>
#include <Utils/Number.h>

#include <random>

namespace CatalystRing {
typedef Observable<void(const Number&), UIComponentPtr> HitObservableBase;
class HitObservable : public HitObservableBase, public Component {
    friend class Catalyst;

   public:
    enum : uint8_t { FUNC = 0, DATA };

    void setPos(const CircleData& pos);

   private:
    void init();

    bool onTimer(Timer& timer);
    void onTimerUpdate(Time dt, Timer& timer);

    std::vector<SubscriptionWPtr> getInRange();

    CircleData mPos;

    TimeSystem::TimerObservable::SubscriptionPtr mTimerSub;

    bool mReady = false;

    std::mt19937 gen = std::mt19937(rand());
    std::uniform_real_distribution<float> rDist;
};

class Service : public ::Service<HitObservable> {};

std::shared_ptr<HitObservable> GetHitObservable();
}  // namespace CatalystRing

#endif
