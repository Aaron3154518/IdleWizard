#ifndef POWER_WIZARD_H
#define POWER_WIZARD_H

#include <Components/Upgrade.h>
#include <Components/WizardBase.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/IconSystem/IconSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Systems/WizardSystem/WizardEvents.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Utils/AnimationData.h>
#include <Wizards/PowerWizard/PowerFireball.h>
#include <Wizards/PowerWizard/PowerWizardConstants.h>
#include <Wizards/TimeWizard/TimeWizardConstants.h>
#include <Wizards/Wizard/WizardConstants.h>
#include <Wizards/Wizard/WizardFireball.h>
#include <Wizards/WizardIds.h>

#include <memory>
#include <vector>

namespace PowerWizard {
class PowerWizard : public WizardBase {
   public:
    PowerWizard();

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
    void onTimeFreeze(bool frozen);

    Number calcPower();
    Number calcSpeed();
    Number calcFBSpeed();
    Number calcFBSpeedEffect();
    void calcTimer();
    Number calcFireRingEffect();

    void shootFireball();
    void shootFireball(SDL_FPoint target);

    WizardId getTarget();
    FireballData newFireballData(WizardId target);

    void setPos(float x, float y);

    TimerObservable::SubscriptionPtr mFireballTimerSub;
    TimeSystem::TimerObservable::SubscriptionPtr mAnimTimerSub;
    WizardSystem::WizardEventObservable::IdSubscriptionPtr mT1ResetSub;
    WizardSystem::HideObservable::AllSubscriptionPtr mTargetHideSub;
    UpgradeList::SubscriptionPtr mPowerDisplay, mPowerUp, mTimeWarpUp;

    FireballPtr mFreezeFireball;
    FireballListPtr mFireballs;
    std::unordered_map<WizardId, int> mTargets;
};
}  // namespace PowerWizard

#endif
