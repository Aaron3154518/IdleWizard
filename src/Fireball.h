#ifndef FIREBALL_H
#define FIREBALL_H

#include <RenderSystem/AssetManager.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <SDL.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <ServiceSystem/EventServices/ResizeService.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Utils/Number.h>
#include <Utils/Time.h>

#include "WizardIds.h"

typedef Observable<void(SDL_FPoint), WizardId> FireballObservableBase;

class FireballObservable : public FireballObservableBase {
   public:
    enum : size_t { FUNC = 0, DATA };

    void next(WizardId id, SDL_FPoint pos);

   private:
    void onSubscribe(SubscriptionPtr sub);

    SDL_FPoint mTargets[size];
};

typedef Observable<void(WizardId, Number), WizardId> TargetObservableBase;

class TargetObservable : public TargetObservableBase {
   public:
    enum : size_t { FUNC = 0, DATA };

    void next(WizardId target, Number val, WizardId src);
};

class FireballService : public Service<FireballObservable, TargetObservable> {};

class Fireball : public Component {
    friend class FireballObservable;

   public:
    Fireball(float cX, float cY, WizardId src, WizardId target, Number val);

    bool dead() const;

    const static std::string IMG;

   private:
    void init();

    void onUpdate(Time dt);

    void onResize(ResizeData data);
    void onRender(SDL_Renderer* renderer);

    bool mDead = false;
    WizardId mSrcId, mTargetId;
    SDL_FPoint mTargetPos{0, 0};
    UIComponentPtr mPos;

    RenderData mImg;

    ResizeObservable::SubscriptionPtr mResizeSub;
    UpdateObservable::SubscriptionPtr mUpdateSub;
    RenderObservable::SubscriptionPtr mRenderSub;
    FireballObservable::SubscriptionPtr mFireballSub;

    Number mVal;

    const static int MAX_SPEED;
    const static int COLLIDE_ERR;
};

#endif