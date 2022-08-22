#ifndef WIZARD_H
#define WIZARD_H

#include <Components/Fireball.h>
#include <Components/Upgrade.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/DragService.h>
#include <ServiceSystem/EventServices/ResizeService.h>
#include <ServiceSystem/ServiceSystem.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Systems/TimeSystem.h>
#include <Systems/WizardSystem.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

class Wizard : public WizardBase {
   public:
    Wizard();

    const static std::string POWER_UP_IMG, SPEED_UP_IMG, MULTI_UP_IMG,
        POWER_BKGRND, FIREBALL_IMG, FIREBALL_BUFFED_IMG, CRIT_UP_IMG;

   private:
    void init();
    void setDefaultValues();
    void setSubscriptions();
    void setUpgrades();
    void setParamTriggers();
    void setEventTriggers();

    void onRender(SDL_Renderer* r);
    void onHide(WizardId id, bool hide);
    void onResetT1();
    bool onTimer(Timer& timer);
    void onFireballHit(const Fireball& fireball);
    void onFireballFireRingHit(Fireball& fireball,
                               const Number& fireRingEffect);
    bool onPowWizTimer(Timer& timer);
    void onPowWizTimerUpdate(Time dt, Timer timer);
    void onFreeze(TimeSystem::FreezeType type);
    void onUnfreeze(TimeSystem::FreezeType type);

    void calcPower();
    void calcSpeed();
    void calcTimer();
    void calcCrit();
    void calcCritSpread();

    struct FireballData {
        Number power;
        float sizeFactor;
    };
    FireballData newFireball();

    void shootFireball();
    void shootFireball(SDL_FPoint target);

    void setPos(float x, float y);

    RenderData mPowBkgrnd;

    TimerObservable::SubscriptionPtr mFireballTimerSub;
    TimeSystem::TimerObservable::SubscriptionPtr mPowWizTimerSub;
    Fireball::HitObservable::IdSubscriptionPtr mFireballSub;
    Fireball::FireRingHitObservable::IdSubscriptionPtr mFireballFireRingSub;
    TimeSystem::FreezeObservable::SubscriptionPtr mFreezeSub;
    UpgradeList::SubscriptionPtr mPowerDisplay, mTargetUp, mPowerUp, mMultiUp,
        mCritUp;

    // Pairs of effect, duration
    std::vector<std::pair<Number, Number>> mPowWizBoosts;

    WizardId mTarget = CRYSTAL;

    std::vector<FireballPtr> mFireballs;
    int mFireballFreezeCnt;
};

#endif