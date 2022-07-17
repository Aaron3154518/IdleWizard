#ifndef FIREBALL_H
#define FIREBALL_H

#include <RenderSystem/AssetManager.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <SDL.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Utils/Number.h>
#include <Utils/Time.h>

#include "WizardIds.h"

typedef Observable<SDL_FPoint, void(SDL_FPoint), WizardId>
    FireballObservableBase;

class FireballObservable : public FireballObservableBase {
   public:
    SubscriptionPtr subscribe(Subscription::Function func,
                              std::shared_ptr<WizardId> id);
    void updateSubscriptionData(SubscriptionPtr sub,
                                std::shared_ptr<WizardId> id);

    void next(WizardId id, SDL_FPoint pos);

   private:
    SDL_FPoint mTargets[WizardId::size];
};

typedef Observable<Number, void(WizardId, Number), WizardId>
    TargetObservableBase;

class TargetObservable : public TargetObservableBase {
   public:
    void next(WizardId target, Number val, WizardId src);
};

class FireballService : public Service<FireballObservable, TargetObservable> {};

class Fireball : public Component {
    friend class FireballObservable;

   public:
    Fireball(float cX, float cY, WizardId src, WizardId target, Number val);
    ~Fireball() = default;

    bool dead() const;

   private:
    void init();

    void onUpdate(Time dt);

    void onRender(SDL_Renderer* renderer);

    WizardId mSrcId;
    std::shared_ptr<WizardId> mTargetId;
    SDL_FPoint mTargetPos{0, 0};
    UIComponentPtr mComp;

    RenderData mImg;

    UpdateObservable::SubscriptionPtr mUpdateSub;
    RenderObservable::SubscriptionPtr mRenderSub;
    FireballObservable::SubscriptionPtr mFireballSub;

    Number mVal;

    const static int MAX_SPEED;
    const static int COLLIDE_ERR;
    const static std::string IMG;
};

#endif