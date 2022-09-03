#ifndef WIZARD_H
#define WIZARD_H

#include <Components/Fireballs/PowerWizFireball.h>
#include <Components/Fireballs/WizardFireball.h>
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
#include <Utils/AnimationData.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

class Wizard : public WizardBase {
   public:
    Wizard();

    const static AnimationData IMG, POWER_BKGRND;

    const static std::string POWER_UP_IMG, SPEED_UP_IMG, MULTI_UP_IMG,
        CRIT_UP_IMG;

    const static std::vector<WizardId> TARGETS;

    static void setDefaults();

    static RenderDataWPtr GetIcon();

   private:
    void init();
    void setSubscriptions();
    void setUpgrades();
    void setParamTriggers();

    void onRender(SDL_Renderer* r);
    void onHide(bool hide);
    void onTargetHide(WizardId id, bool hide);
    void onT1Reset();
    bool onTimer(Timer& timer);
    void onPowFireballHit(const PowerWizFireball& fireball);
    bool onPowWizTimer(Timer& timer);
    void onPowWizTimerUpdate(Time dt, Timer& timer);
    void onTimeFreeze(bool frozen);
    void onTimeWarp();

    Number calcPower();
    Number calcSpeed();
    Number calcFBSpeed();
    Number calcFBSpeedEffect();
    void calcTimer();
    Number calcCrit();
    Number calcCritSpread();

    WizardFireball::Data newFireballData();

    void shootFireball();
    void shootFireball(SDL_FPoint target);

    void setPos(float x, float y);

    RenderData mPowBkgrnd;

    TimerObservable::SubscriptionPtr mFireballTimerSub;
    TimeSystem::TimerObservable::SubscriptionPtr mPowWizTimerSub, mAnimTimerSub,
        mPowBkAnimTimerSub;
    PowerWizFireball::HitObservable::IdSubscriptionPtr mPowFireballHitSub;
    WizardSystem::HideObservable::AllSubscriptionPtr mTargetHideSub;
    WizardSystem::WizardEventObservable::IdSubscriptionPtr mT1ResetSub,
        mTimeWarpSub;
    UpgradeList::SubscriptionPtr mPowerDisplay, mTargetUp, mPowerUp, mMultiUp,
        mCritUp;

    // Pairs of effect, duration
    std::vector<std::pair<Number, Number>> mPowWizBoosts;

    WizardId mTarget = CRYSTAL;

    WizardFireballPtr mFreezeFireball;
    std::vector<WizardFireballPtr> mFireballs;
};

#endif