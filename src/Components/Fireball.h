#ifndef FIREBALL_H
#define FIREBALL_H

#include <Components/FireRing.h>
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
#include <Systems/TargetSystem.h>
#include <Systems/TimeSystem.h>
#include <Utils/Number.h>
#include <Utils/Time.h>
#include <Wizards/WizardIds.h>
#include <Wizards/WizardTypes.h>

#include <memory>

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

class Fireball : public Component {
    friend class FireballObservable;

   public:
    typedef TargetSystem::TargetObservable<const Fireball&> HitObservable;
    typedef TargetSystem::TargetObservable<Fireball&, const Number&>
        FireRingHitObservable;

   public:
    const static Rect IMG_RECT;
    const static int DEF_VALUE_KEY;

    Fireball(SDL_FPoint c, WizardId src, WizardId target,
             const std::string& img, const Number& val);
    Fireball(SDL_FPoint c, WizardId src, WizardId target,
             const std::string& img, const NumberMap<int>& vals);

    bool dead() const;

    void launch(SDL_FPoint target);

    void setSize(float size);
    void setPos(float x, float y);

    Number& getValue(int key = DEF_VALUE_KEY);
    const Number& getValue(int key = DEF_VALUE_KEY) const;

    WizardId getSourceId() const;
    WizardId getTargetId() const;

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

    NumberMap<int> mVals;

    RenderData mImg;

    float mSize = 1;

    ResizeObservable::SubscriptionPtr mResizeSub;
    TimeSystem::UpdateObservable::SubscriptionPtr mUpdateSub;
    RenderObservable::SubscriptionPtr mRenderSub;
    FireballObservable::SubscriptionPtr mFireballSub;
    FireRing::HitObservable::SubscriptionPtr mFireRingSub;

    const static int COLLIDE_ERR, MAX_SPEED, ACCELERATION, ACCEL_ZONE;
};

typedef std::unique_ptr<Fireball> FireballPtr;

class FireballService
    : public Service<FireballObservable, Fireball::HitObservable,
                     Fireball::FireRingHitObservable> {};

#endif