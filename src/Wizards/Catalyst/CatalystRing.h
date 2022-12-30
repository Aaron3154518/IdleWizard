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
#include <Systems/WizardSystem/WizardObservables.h>
#include <Utils/Number.h>
#include <Wizards/Catalyst/CatalystParameters.h>

#include <random>

namespace Catalyst {
namespace Ring {
typedef Observable<void(bool), UIComponentPtr, int> HitObservableBase;
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

    enum : uint8_t { FUNC = 0, DATA, ZAP_CNT };

    HitObservable();

    SubscriptionPtr subscribe(std::function<void()> func, UIComponentPtr pos);
    SubscriptionPtr subscribe(std::function<void(bool)> func,
                              UIComponentPtr pos);

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
    WizardSystem::HideObservable::IdSubscriptionPtr mCatHideSub;
    ParameterSystem::ParameterSubscriptionPtr mNumZapsSub;

    int mReady, mMaxZaps;

    std::vector<std::unique_ptr<Zap>> mZaps;

    std::mt19937 gen = std::mt19937(rand());
    std::uniform_real_distribution<float> rDist;
};

std::shared_ptr<HitObservable> GetHitObservable();

class Service : public ::Service<HitObservable> {};
}  // namespace Ring
}  // namespace Catalyst

#endif
