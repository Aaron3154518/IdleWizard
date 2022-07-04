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
#include <Utils/Time.h>

#include "WizardIds.h"

typedef Observable<SDL_Point, void(SDL_Point), WizardId> FireballObservableBase;

class FireballObservable : public FireballObservableBase {
   public:
    SubscriptionPtr subscribe(Subscription::Function func, std::shared_ptr<WizardId> id);
    void updateSubscriptionData(SubscriptionPtr sub, std::shared_ptr<WizardId> id);

    void next(WizardId id, SDL_Point pos);

   private:
    SDL_Point mTargets[WizardId::size];
};

class FireballService : public Service<FireballObservable> {};

class Fireball : public Component {
    friend class FireballObservable;

   public:
    Fireball(SDL_Point c, WizardId target);
    ~Fireball() = default;

    bool dead() const;

   private:
    void init();

    void setPos(float x, float y);

    void onUpdate(Time dt);

    void onRender(SDL_Renderer* renderer);

    std::shared_ptr<WizardId> mTargetId;
    SDL_Point mTargetPos{0, 0};
    SDL_FPoint mPos;
    UIComponentPtr mComp;

    RenderData mImg;

    UpdateObservable::SubscriptionPtr mUpdateSub;
    RenderObservable::SubscriptionPtr mRenderSub;
    FireballObservable::SubscriptionPtr mFireballSub;

    const static int MAX_SPEED;
    const static int COLLIDE_ERR;
    const static std::string IMG;
};

#endif