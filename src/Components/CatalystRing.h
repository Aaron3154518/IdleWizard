#ifndef CATALYST_RING_H
#define CATALYST_RING_H

#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
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
    struct Zap : public Component {
       public:
        Zap(SDL_Point p1, SDL_Point p2);

        bool dead() const;

        SDL_Point mP1, mP2;

       private:
        void init();

        void onRender(SDL_Renderer* r);
        bool onTimer(Timer& timer);

        RenderObservable::SubscriptionPtr mRenderSub;
        TimeSystem::TimerObservable::SubscriptionPtr mTimerSub;
    };

    enum : uint8_t { FUNC = 0, DATA };

    HitObservable();

    void setPos(const CircleData& circle);

    const static std::string ZAP_IMG;

   private:
    void init();

    void onRender(SDL_Renderer* r);
    bool onTimer(Timer& timer);
    void onTimerUpdate(Time dt, Timer& timer);

    std::vector<SubscriptionWPtr> getInRange();

    UIComponentPtr mPos;
    CircleData mCircle;

    RenderObservable::SubscriptionPtr mRenderSub;
    TimeSystem::TimerObservable::SubscriptionPtr mTimerSub;

    bool mReady = false;

    std::vector<std::unique_ptr<Zap>> mZaps;

    std::mt19937 gen = std::mt19937(rand());
    std::uniform_real_distribution<float> rDist;
};

class Service : public ::Service<HitObservable> {};

std::shared_ptr<HitObservable> GetHitObservable();
}  // namespace CatalystRing

#endif
