#ifndef FIREBALL_H
#define FIREBALL_H

#include <Components/Fireballs/Glob.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <SDL.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/ResizeService.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/IconSystem.h>
#include <Systems/TimeSystem.h>
#include <Systems/WizardSystem.h>
#include <Utils/AnimationData.h>
#include <Utils/Time.h>
#include <Wizards/WizardIds.h>

#include <memory>

class Fireball : public Component {
    friend class Fireballs;

   public:
    const static Rect IMG_RECT;
    const static int COLLIDE_ERR, MAX_SPEED;

    Fireball(SDL_FPoint c, WizardId target, float maxSpeedMult);
    virtual ~Fireball() = default;

    void launch(SDL_FPoint target);

    float getSize() const;
    void setSize(float size);

    float getSpeed() const;
    void setSpeed(float speed);

    void setPos(float x, float y);

    void setImg(const RenderTextureCPtr& img);

    WizardId getTargetId() const;

   private:
    virtual void init();

    virtual bool onUpdate(Time dt);
    virtual void onDeath();

    const WizardId mTargetId;

    float mSize = 1, mMaxSpeed;
    UIComponentPtr mPos;
    SDL_FPoint mV{0, 0}, mA{0, 0};

    RenderData mImg;

    Glob::HitObservable::SubscriptionPtr mGlobHitSub;
};

typedef std::unique_ptr<Fireball> FireballPtr;

class Fireballs : public Component {
    friend class FireballObservable;

   public:
    virtual ~Fireballs() = default;

    void add(FireballPtr fb);
    void add(FireballPtr fb, const std::string& img);
    void add(FireballPtr fb, const AnimationData& img);

   protected:
    virtual void init();

    virtual void onUpdate(Time dt);
    virtual void onResize(ResizeData data);
    virtual void onRender(SDL_Renderer* renderer);

    IconSystem::AnimationManager mAnims;

    ResizeObservable::SubscriptionPtr mResizeSub;
    TimeSystem::UpdateObservable::SubscriptionPtr mUpdateSub;
    RenderObservable::SubscriptionPtr mRenderSub;

    std::vector<FireballPtr> mFireballs;
};

typedef std::unique_ptr<Fireballs> FireballsPtr;

#endif