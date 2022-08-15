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
#include <ServiceSystem/Lockable.h>
#include <ServiceSystem/ServiceSystem.h>
#include <ServiceSystem/UpdateServices/TimerService.h>

#include <memory>
#include <random>
#include <string>
#include <vector>

#include "FireRing.h"
#include "Fireball.h"
#include "TimeSystem.h"
#include "Upgrade.h"
#include "WizardBase.h"
#include "WizardData.h"
#include "WizardIds.h"
#include "WizardTypes.h"

class Wizard : public WizardBase {
   public:
    Wizard();

    const static std::string POWER_UP_IMG, SPEED_UP_IMG, MULTI_UP_IMG,
        POWER_BKGRND, FIREBALL_IMG;

   private:
    void init();

    void onRender(SDL_Renderer* r);
    bool onTimer();
    void onHit(WizardId src, const Number& val);

    void calcPower();
    void calcSpeed();
    void calcTimer();

    std::unique_ptr<Fireball>& shootFireball();

    RenderData mPowBkgrnd;

    TimerObservable::SubscriptionPtr mFireballTimerSub;
    TimeSystem::TimerObservable::SubscriptionPtr mPowWizTimerSub;
    TargetObservable::SubscriptionPtr mTargetSub;
    UpgradeList::SubscriptionPtr mPowerDisplay, mTargetUp, mPowerUp, mMultiUp;

    WizardId mTarget = CRYSTAL;

    std::vector<std::unique_ptr<Fireball>> mFireballs;
};

class Crystal : public WizardBase {
   public:
    Crystal();

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onClick(Event::MouseButton b, bool clicked);
    void onHit(WizardId src, const Number& val);

    void calcMagicEffect();
    void drawMagic();

    std::unique_ptr<FireRing>& createFireRing();

    TargetObservable::SubscriptionPtr mTargetSub;
    UpgradeList::SubscriptionPtr mMagicEffectDisplay;

    std::vector<std::unique_ptr<FireRing>> mFireRings;

    TextRenderData mMagicText;
};

class Catalyst : public WizardBase {
   public:
    Catalyst();

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onHit(WizardId src, Number val);

    void calcMagicEffect();
    void drawMagic();

    TargetObservable::SubscriptionPtr mTargetSub;
    UpgradeList::SubscriptionPtr mMagicEffectDisplay;

    TextRenderData mMagicText;
};

class PowerWizard : public WizardBase {
   public:
    PowerWizard();

    const static std::string FIREBALL_IMG;

   private:
    void init();

    void onRender(SDL_Renderer* r);
    bool onTimer();

    void calcPower();
    void calcSpeed();
    void calcTimer();
    void calcFireRingEffect();

    std::unique_ptr<Fireball>& shootFireball();

    TimerObservable::SubscriptionPtr mFireballTimerSub;
    UpgradeList::SubscriptionPtr mPowerDisplay;

    std::vector<std::unique_ptr<Fireball>> mFireballs;
};

class TimeWizard : public WizardBase {
   public:
    TimeWizard();

    const static std::string TIME_WIZ_ACTIVE, TIME_WIZ_FREEZE;

   private:
    void init();

    void onUpdate(Time dt);
    bool startFreeze();
    bool endFreeze();

    void calcCost();

    void updateImg();

    bool mActive = false, mCanAfford = false;

    UpdateObservable::SubscriptionPtr mUpdateSub;
    TimerObservable::SubscriptionPtr mFreezeDelaySub, mFreezeTimerSub;
    UpgradeList::SubscriptionPtr mEffectDisplay, mActiveUp;

    Lock mFreezeLock;

    std::vector<std::unique_ptr<Fireball>> mFireballs;
};

#endif