#ifndef FIRE_RING_H
#define FIRE_RING_H

#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderSystem.h>
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

class FireRing : public Component {
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
    UpdateObservable::SubscriptionPtr mUpdateSub;
};

typedef Observable<void(const Number&), UIComponentPtr> FireRingObservableBase;
class FireRingObservable : public FireRingObservableBase {
   public:
    enum : size_t { FUNC = 0, DATA };

    void next(SDL_FPoint c, int r, const Number& effect);
};

class FireRingService : public Service<FireRingObservable> {};

#endif
