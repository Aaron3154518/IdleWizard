#ifndef TIME_WIZARD_H
#define TIME_WIZARD_H

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
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>

#include <memory>

class TimeWizard : public WizardBase {
   public:
    TimeWizard();

    const static AnimationData IMG, FREEZE_IMG;

    const static std::string FREEZE_UP_IMG, SPEED_UP_IMG;

    static void setDefaults();

   private:
    void init();
    void setSubscriptions();
    void setUpgrades();
    void setParamTriggers();

    bool onCostTimer(Timer& timer);
    void onRender(SDL_Renderer*);
    void onHide(WizardId id, bool hide);
    void onReset(WizardSystem::ResetTier tier);
    bool startFreeze(Timer& timer);
    void onFreezeTimer(Time dt, Timer& timer);
    bool endFreeze(Timer& timer);
    void startFreezeCycle();

    Number calcFreezeEffect();
    Number calcSpeedEffect();
    Number calcCost();
    Number calcClockSpeed();

    void updateImg();

    void setPos(float x, float y);

    bool mActive = false;

    TimerObservable::SubscriptionPtr mCostTimerSub, mFreezeDelaySub,
        mFreezeTimerSub, mAnimTimerSub;
    UpgradeList::SubscriptionPtr mEffectDisplay, mActiveUp, mFBSpeedUp,
        mFreezeUp, mSpeedUp;

    TogglePtr mActiveToggle;
    Lock mTimeLock;

    ProgressBar mFreezePb;

    std::unique_ptr<TimeWizClock> mTClock;

    AnimationData mImgAnimData = IMG;
};

#endif
