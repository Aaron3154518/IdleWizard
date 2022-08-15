#ifndef FIREBALL_H
#define FIREBALL_H

#include <RenderSystem/AssetManager.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <SDL.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/ResizeService.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Utils/Number.h>
#include <Utils/Time.h>

#include "FireRing.h"
#include "TimeSystem.h"
#include "WizardIds.h"

typedef Observable<void(SDL_FPoint), WizardId> FireballObservableBase;

class FireballObservable : public FireballObservableBase {
   public:
    enum : size_t { FUNC = 0, DATA };

    void next(WizardId id, SDL_FPoint pos);

    SDL_FPoint getPos(WizardId id) const;

   private:
    void onSubscribe(SubscriptionPtr sub);

    SDL_FPoint mTargets[WizardId::size];
};

typedef Observable<void(WizardId, const Number&), WizardId>
    TargetObservableBase;

class TargetObservable : public TargetObservableBase {
   public:
    enum : size_t { FUNC = 0, DATA };

    void next(WizardId target, const Number& val, WizardId src);
};

class FireballService : public Service<FireballObservable, TargetObservable> {};

class Fireball : public Component {
    friend class FireballObservable;

   public:
    Fireball(SDL_FPoint c, WizardId src, WizardId target, Number val,
             const std::string& img);

    bool dead() const;

    void launch(SDL_FPoint target);

   private:
    void init();

    void onUpdate(Time dt);
    void onResize(ResizeData data);
    void onRender(SDL_Renderer* renderer);
    void onFireRing(const Number& effect);

    bool mDead = false, mHitFireRing = false;
    WizardId mSrcId, mTargetId;
    SDL_FPoint mTargetPos{0, 0}, mV{0, 0}, mA{0, 0};
    UIComponentPtr mPos;

    RenderData mImg;

    ResizeObservable::SubscriptionPtr mResizeSub;
    TimeSystem::UpdateObservable::SubscriptionPtr mUpdateSub;
    RenderObservable::SubscriptionPtr mRenderSub;
    FireballObservable::SubscriptionPtr mFireballSub;
    FireRingObservable::SubscriptionPtr mFireRingSub;

    Number mVal;

    const static int COLLIDE_ERR, MAX_SPEED, ACCELERATION, ACCEL_ZONE;
};

#endif