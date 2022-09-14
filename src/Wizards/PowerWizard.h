#ifndef POWER_WIZARD_H
#define POWER_WIZARD_H

#include <Components/Fireballs/PowerFireball.h>
#include <Components/Fireballs/WizardFireball.h>
#include <Components/Upgrade.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/IconSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Systems/WizardSystem.h>
#include <Utils/AnimationData.h>
#include <Wizards/Definitions/PowerWizardDefs.h>
#include <Wizards/Definitions/TimeWizardDefs.h>
#include <Wizards/Definitions/WizardDefs.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>

#include <memory>
#include <vector>

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
    PowerFireball::Data newFireballData(WizardId target);

    void setPos(float x, float y);

    TimerObservable::SubscriptionPtr mFireballTimerSub;
    TimeSystem::TimerObservable::SubscriptionPtr mAnimTimerSub;
    WizardSystem::WizardEventObservable::IdSubscriptionPtr mT1ResetSub;
    WizardSystem::HideObservable::AllSubscriptionPtr mTargetHideSub;
    UpgradeList::SubscriptionPtr mPowerDisplay, mPowerUp, mTimeWarpUp;

    PowerFireballPtr mFreezeFireball;
    FireballListPtr<PowerFireball> mFireballs;
    std::unordered_map<WizardId, int> mTargets;
};

#endif
