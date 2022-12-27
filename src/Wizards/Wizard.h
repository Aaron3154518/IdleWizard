#ifndef WIZARD_H
#define WIZARD_H

#include <Components/Fireballs/Glob.h>
#include <Components/Fireballs/PowerFireball.h>
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
#include <Systems/IconSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Systems/TimeSystem.h>
#include <Systems/WizardSystem/WizardEvents.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Utils/AnimationData.h>
#include <Wizards/Definitions/WizardDefs.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

class Wizard : public WizardBase {
   public:
    Wizard();

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
    void onPowFireballHit(const PowerFireball& fireball);
    void onGlobHit();
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

    void shootFireball(const WizardFireball::Data& data);
    void shootFireball(const WizardFireball::Data& data, SDL_FPoint launch);

    void setPos(float x, float y);

    RenderAnimation mPowBkgrnd;

    TimerObservable::SubscriptionPtr mFireballTimerSub;
    TimeSystem::TimerObservable::SubscriptionPtr mPowWizTimerSub, mAnimTimerSub,
        mPowBkAnimTimerSub;
    PowerFireballList::HitObservable::SubscriptionPtr mPowFireballHitSub;
    WizardSystem::HideObservable::AllSubscriptionPtr mTargetHideSub;
    WizardSystem::WizardEventObservable::IdSubscriptionPtr mT1ResetSub,
        mTimeWarpSub;
    UpgradeList::SubscriptionPtr mPowerDisplay, mTargetUp, mPowerUp, mMultiUp,
        mCritUp, mRoboCritUp;
    Glob::HitObservable::SubscriptionPtr mGlobHitSub;

    // Pairs of effect, duration
    std::vector<std::pair<Number, Number>> mPowWizBoosts;

    WizardId mTarget = CRYSTAL;

    WizardFireballPtr mFreezeFireball;
    WizardFireballListPtr mFireballs;
};

#endif