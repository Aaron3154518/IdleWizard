#ifndef FIRE_RING_H
#define FIRE_RING_H

#include <Components/FireballHit.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderSystem.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <SDL.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/TargetSystem.h>
#include <Systems/TimeSystem.h>
#include <Utils/Number.h>
#include <Utils/Time.h>
#include <Wizards/WizardIds.h>

#include <typeindex>
#include <unordered_map>

class FireRing : public Component {
   public:
    typedef Observable<void(const Number&), UIComponentPtr>
        FireballObservableBase;
    class FireballObservable : public FireballObservableBase {
        friend class RingObservable;

       public:
        enum : uint8_t { FUNC = 0, DATA };

        FireballObservable(SDL_Point c, int r);

        void next(SDL_Point c, int r, const Number& effect);

        void subscribe(SubscriptionPtr sub);

       private:
        SDL_Point mC;
        int mR;
    };

    typedef FireballHitObservable<FireballObservable> HitObservable;

   public:
    FireRing(SDL_Point c, const Number& effect);

    bool dead() const;

    const static int GROWTH, WIDTH;
    const static SDL_Color COLOR;

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onUpdate(Time dt);

    bool mDead = false;

    CircleShape mCircle;
    Number mEffect;

    RenderObservable::SubscriptionPtr mRenderSub;
    TimeSystem::UpdateObservable::SubscriptionPtr mUpdateSub;
    HitObservable::FireballsSubscriptionPtr mRingSub;
};

class FireRingService
    : public Service<FireRing::HitObservable, FireRing::HitObservable> {};

#endif
