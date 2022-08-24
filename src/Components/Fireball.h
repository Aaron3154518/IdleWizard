#ifndef FIREBALL_H
#define FIREBALL_H

#include <Components/FireRing.h>
#include <Components/FireballService.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <SDL.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/ResizeService.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Systems/TargetSystem.h>
#include <Systems/TimeSystem.h>
#include <Utils/Number.h>
#include <Utils/Time.h>
#include <Wizards/Catalyst.h>
#include <Wizards/WizardIds.h>

#include <array>
#include <memory>

class Fireball : public Component {
    friend class FireballObservable;

   public:
    enum State : uint8_t {
        HitFireRing = 0,
        PowerWizBoosted,

        size
    };

   public:
    const static Rect IMG_RECT;
    const static int DEF_VALUE_KEY;

    Fireball(SDL_FPoint c, WizardId src, WizardId target,
             const std::string& img, const Number& val);
    Fireball(SDL_FPoint c, WizardId src, WizardId target,
             const std::string& img, const NumberMap& vals);

    bool dead() const;

    void launch(SDL_FPoint target);

    float getSize() const;
    void setSize(float size);

    void setPos(float x, float y);

    bool getState(State state) const;
    bool& getState(State state);

    const Number& getValue(int key = DEF_VALUE_KEY) const;
    Number& getValue(int key = DEF_VALUE_KEY);

    WizardId getSourceId() const;
    WizardId getTargetId() const;

   private:
    void init();

    void onUpdate(Time dt);
    void onResize(ResizeData data);
    void onRender(SDL_Renderer* renderer);
    void onFireRing(const Number& effect);
    void onCatalyst(const Number& effect);

    bool mDead = false;
    const WizardId mSrcId, mTargetId;
    SDL_FPoint mTargetPos{0, 0}, mV{0, 0}, mA{0, 0};
    UIComponentPtr mPos;

    std::array<bool, State::size> mState;
    NumberMap mVals;

    RenderData mImg;

    float mSize = 1;

    ResizeObservable::SubscriptionPtr mResizeSub;
    TimeSystem::UpdateObservable::SubscriptionPtr mUpdateSub;
    RenderObservable::SubscriptionPtr mRenderSub;
    FireballObservable::SubscriptionPtr mFireballSub;
    FireRing::HitObservable::SubscriptionPtr mFireRingSub;
    Catalyst::HitObservable::SubscriptionPtr mCatalystSub;

    const static int COLLIDE_ERR, MAX_SPEED, ACCELERATION, ACCEL_ZONE;
};

typedef std::unique_ptr<Fireball> FireballPtr;

#endif