#ifndef FIREBALL_H
#define FIREBALL_H

#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <SDL.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/ResizeService.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/TimeSystem.h>
#include <Systems/WizardSystem.h>
#include <Utils/AnimationData.h>
#include <Utils/Time.h>
#include <Wizards/WizardIds.h>

#include <memory>

class Fireball : public Component {
    friend class FireballObservable;

   public:
    const static Rect IMG_RECT;

    Fireball(SDL_FPoint c, WizardId target, const std::string& img);
    Fireball(SDL_FPoint c, WizardId target, const AnimationData& img);
    virtual ~Fireball() = default;

    bool dead() const;

    void launch(SDL_FPoint target);

    float getSize() const;
    void setSize(float size);

    void setPos(float x, float y);

    WizardId getTargetId() const;

   protected:
    virtual void init();

    void onUpdate(Time dt);
    void onResize(ResizeData data);
    void onRender(SDL_Renderer* renderer);

    virtual void onDeath();

    bool mDead = false;
    const WizardId mTargetId;
    SDL_FPoint mTargetPos{0, 0}, mV{0, 0}, mA{0, 0};
    UIComponentPtr mPos;

    RenderData mImg;
    AnimationData mImgAnim;

    float mSize = 1;

    ResizeObservable::SubscriptionPtr mResizeSub;
    TimerObservable::SubscriptionPtr mAnimTimerSub;
    TimeSystem::UpdateObservable::SubscriptionPtr mUpdateSub;
    RenderObservable::SubscriptionPtr mRenderSub;
    WizardSystem::WizardPosObservable::IdSubscriptionPtr mTargetSub;

    const static int COLLIDE_ERR, MAX_SPEED;
};

typedef std::unique_ptr<Fireball> FireballPtr;

#endif