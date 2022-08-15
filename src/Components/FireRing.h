#ifndef FIRE_RING_H
#define FIRE_RING_H

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

class FireRing : public Component {
   public:
    typedef Observable<void(const Number&), UIComponentPtr> HitObservableBase;
    class HitObservable : public HitObservableBase {
       public:
        enum : size_t { FUNC = 0, DATA };

        void next(SDL_FPoint c, int r, const Number& effect);
    };

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

    CircleData mCircle;
    Number mEffect;

    RenderObservable::SubscriptionPtr mRenderSub;
    TimeSystem::UpdateObservable::SubscriptionPtr mUpdateSub;
};

class FireRingService : public Service<FireRing::HitObservable> {};

#endif
