#ifndef TIME_WIZARD_H
#define TIME_WIZARD_H

#include <Components/Upgrade.h>
#include <Components/WizardBase.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <ServiceSystem/Lockable.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/IconSystem/IconSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/TimeSystem.h>
#include <Systems/WizardSystem/WizardEvents.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Utils/AnimationData.h>
#include <Wizards/Crystal/CrystalParameters.h>
#include <Wizards/PoisonWizard/Glob.h>
#include <Wizards/PowerWizard/PowerFireball.h>
#include <Wizards/PowerWizard/PowerWizardConstants.h>
#include <Wizards/PowerWizard/PowerWizardParameters.h>
#include <Wizards/TimeWizard/TimeWizClock.h>
#include <Wizards/TimeWizard/TimeWizardConstants.h>
#include <Wizards/TimeWizard/TimeWizardParameters.h>
#include <Wizards/Wizard/WizardConstants.h>
#include <Wizards/Wizard/WizardFireball.h>
#include <Wizards/WizardIds.h>

#include <memory>

namespace TimeWizard {
class TimeWizard : public WizardBase {
   public:
    TimeWizard();

   private:
    void init();
    void setSubscriptions();
    void setUpgrades();
    void setParamTriggers();

    bool onCostTimer(Timer& timer);
    void onRender(SDL_Renderer*);
    void onHide(bool hide);
    void onT1Reset();
    void onFreezeChange(bool frozen);
    void onPowFireballHit(const PowerWizard::Fireball& fireball);
    bool powFireballFilter(const PowerWizard::Fireball& fireball);
    void onGlobHit();

    Number calcFreezeEffect();
    Number calcSpeedEffect();
    Number calcCost();
    Number calcClockSpeed();

    void updateImg();

    void setPos(float x, float y);

    bool mActive = false;

    WizardSystem::WizardEventObservable::IdSubscriptionPtr mT1ResetSub;
    PowerWizard::FireballList::HitObservable::SubscriptionPtr
        mPowFireballHitSub;
    TimerObservable::SubscriptionPtr mCostTimerSub, mFreezeDelaySub,
        mFreezeTimerSub, mAnimTimerSub;
    UpgradeList::SubscriptionPtr mEffectDisplay, mActiveUp, mFBSpeedUp,
        mFreezeUp, mSpeedUp, mSpeedUpUp;
    PoisonWizard::Glob::HitObservable::SubscriptionPtr mGlobHitSub;

    TogglePtr mActiveToggle;
    Lock mTimeLock;

    ProgressBar mFreezePb;

    std::unique_ptr<Clock> mTClock;

    AnimationData mImgAnimData;
};
}  // namespace TimeWizard

#endif
