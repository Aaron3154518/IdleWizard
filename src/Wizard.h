#ifndef WIZARD_H
#define WIZARD_H

#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/DragService.h>
#include <ServiceSystem/EventServices/MouseService.h>
#include <ServiceSystem/EventServices/ResizeService.h>
#include <ServiceSystem/ServiceSystem.h>
#include <ServiceSystem/UpdateServices/TimerService.h>

#include <memory>
#include <random>
#include <vector>

#include "Fireball.h"
#include "Upgrade.h"
#include "WizardIds.h"
#include "WizardUpdate.h"

class WizardBase : public Component {
   public:
    virtual ~WizardBase();

    const static Rect IMG_RECT;
    const static std::string IMGS[];
    const static FontData FONT;

   protected:
    WizardBase(WizardId id);

    virtual void init();

    virtual void onResize(ResizeData data);
    virtual void onRender(SDL_Renderer* r);
    virtual void onClick(Event::MouseButton b, bool clicked);

    void setPos(float x, float y);

    void setImage(const std::string& img);

    const WizardId mId;

    RenderData mImg;

    DragComponentPtr mComp;
    ResizeObservable::SubscriptionPtr mResizeSub;
    RenderObservable::SubscriptionPtr mRenderSub;
    MouseObservable::SubscriptionPtr mMouseSub;
    DragObservable::SubscriptionPtr mDragSub;

    std::mt19937 gen = std::mt19937(rand());
    std::uniform_int_distribution<> dist =
        std::uniform_int_distribution<>(1, WizardId::size - 1);
    std::uniform_real_distribution<> rDist;
};

class Wizard : public WizardBase {
   public:
    Wizard();

    const static std::string POWER_UP_IMG, SPEED_UP_IMG;

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onClick(Event::MouseButton b, bool clicked);
    bool onTimer();
    void onWizardUpdate(const ParameterList<WizardParams>& params);

    void shootFireball();

    TimerObservable::SubscriptionPtr mTimerSub;
    WizardParameters::SubscriptionPtr mWizUpdateSub;

    WizardId mTarget = WizardId::CRYSTAL;
    std::vector<std::shared_ptr<Upgrade>> mUpgrades;
    bool mPowerBought = false;
    uint8_t mSpeedBoughtCnt = 0;

    Number mBasePower = Number(1), mPower = Number(1);
    std::vector<std::unique_ptr<Fireball>> mFireballs;
};

class Crystal : public WizardBase {
   public:
    Crystal();

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onHit(WizardId src, Number val);

    TargetObservable::SubscriptionPtr mTargetSub;

    TextRenderData mMagicText;
    Number mMagic;
};

class Catalyst : public WizardBase {
   public:
    Catalyst();

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onHit(WizardId src, Number val);

    TargetObservable::SubscriptionPtr mTargetSub;

    TextRenderData mMagicText;
    Number mMagic, mCapacity = Number(100);
};

#endif