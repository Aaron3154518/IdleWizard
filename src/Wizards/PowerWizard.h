#ifndef POWER_WIZARD_H
#define POWER_WIZARD_H

#include <Components/Fireballs/PowerWizFireball.h>
#include <Components/Upgrade.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Systems/WizardSystem.h>
#include <Utils/AnimationData.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>

#include <memory>
#include <vector>

class PowerWizard : public WizardBase {
   public:
    PowerWizard();

    const static AnimationData IMG;
    const static std::string POWER_UP_IMG;

    static void setDefaults();

   private:
    void init();
    void setSubscriptions();
    void setUpgrades();
    void setParamTriggers();

    void onRender(SDL_Renderer* r);
    void onHide(WizardId id, bool hide);
    void onReset(WizardSystem::ResetTier tier);
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

    PowerWizFireball::Data newFireballData(WizardId target);

    void setPos(float x, float y);

    TimerObservable::SubscriptionPtr mFireballTimerSub;
    TimeSystem::TimerObservable::SubscriptionPtr mAnimTimerSub;
    UpgradeList::SubscriptionPtr mPowerDisplay, mPowerUp;

    PowerWizFireballPtr mFreezeFireball;
    std::vector<PowerWizFireballPtr> mFireballs;
    int mWizTargetCnt = 0, mCrysTargetCnt = 0;
};

#endif
