#ifndef WIZARD_H
#define WIZARD_H

#include <Components/Upgrade.h>
#include <Components/WizardBase.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/DragService.h>
#include <ServiceSystem/EventServices/ResizeService.h>
#include <ServiceSystem/ServiceSystem.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/IconSystem/IconSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Systems/TimeSystem.h>
#include <Systems/WizardSystem/WizardEvents.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Utils/AnimationData.h>
#include <Wizards/PoisonWizard/Glob.h>
#include <Wizards/PowerWizard/PowerFireball.h>
#include <Wizards/Wizard/WizardConstants.h>
#include <Wizards/Wizard/WizardFireball.h>
#include <Wizards/WizardIds.h>

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

namespace Wizard {
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
    void onPowFireballHit(const PowerWizard::Fireball& fireball);
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

    Fireball::Data newFireballData();

    void shootFireball(const Fireball::Data& data);
    void shootFireball(const Fireball::Data& data, SDL_FPoint launch);

    void setPos(float x, float y);

    RenderAnimation mPowBkgrnd;

    TimerObservable::SubscriptionPtr mFireballTimerSub;
    TimeSystem::TimerObservable::SubscriptionPtr mPowWizTimerSub, mAnimTimerSub,
        mPowBkAnimTimerSub;
    PowerWizard::FireballList::HitObservable::SubscriptionPtr
        mPowFireballHitSub;
    WizardSystem::HideObservable::AllSubscriptionPtr mTargetHideSub;
    WizardSystem::WizardEventObservable::IdSubscriptionPtr mT1ResetSub,
        mTimeWarpSub;
    UpgradeList::SubscriptionPtr mPowerDisplay, mTargetUp, mPowerUp, mMultiUp,
        mCritUp, mRoboCritUp;
    PoisonWizard::Glob::HitObservable::SubscriptionPtr mGlobHitSub;

    // Pairs of effect, duration
    std::vector<std::pair<Number, Number>> mPowWizBoosts;

    WizardId mTarget = CRYSTAL;

    FireballPtr mFreezeFireball;
    FireballListPtr mFireballs;
};
}  // namespace Wizard

#endif