#ifndef TIME_WIZARD_H
#define TIME_WIZARD_H

#include <Components/Fireballs/PowerWizFireball.h>
#include <Components/Fireballs/WizardFireball.h>
#include <Components/TimeWizClock.h>
#include <Components/Upgrade.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <ServiceSystem/Lockable.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Systems/TimeSystem.h>
#include <Systems/WizardSystem.h>
#include <Utils/AnimationData.h>
#include <Wizards/Definitions/PowerWizardDefs.h>
#include <Wizards/Definitions/TimeWizardDefs.h>
#include <Wizards/Definitions/WizardDefs.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>

#include <memory>

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
    void onPowFireballHit(const PowerWizFireball& fireball);

    Number calcFreezeEffect();
    Number calcSpeedEffect();
    Number calcCost();
    Number calcClockSpeed();

    void updateImg();

    void setPos(float x, float y);

    bool mActive = false;

    WizardSystem::WizardEventObservable::IdSubscriptionPtr mT1ResetSub;
    PowerWizFireball::HitObservable::IdSubscriptionPtr mPowFireballHitSub;
    TimerObservable::SubscriptionPtr mCostTimerSub, mFreezeDelaySub,
        mFreezeTimerSub, mAnimTimerSub;
    UpgradeList::SubscriptionPtr mEffectDisplay, mActiveUp, mFBSpeedUp,
        mFreezeUp, mSpeedUp, mSpeedUpUp;

    TogglePtr mActiveToggle;
    Lock mTimeLock;

    ProgressBar mFreezePb;

    std::unique_ptr<TimeWizClock> mTClock;

    AnimationData mImgAnimData = TimeWizardDefs::IMG;
};

#endif
