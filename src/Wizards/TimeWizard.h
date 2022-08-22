#ifndef TIME_WIZARD_H
#define TIME_WIZARD_H

#include <Components/Upgrade.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <ServiceSystem/Lockable.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Systems/WizardSystem.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>

#include <memory>

class TimeWizard : public WizardBase {
   public:
    TimeWizard();

    const static std::string ACTIVE_IMG, FREEZE_IMG, FREEZE_UP_IMG,
        SPEED_UP_IMG;

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
    bool endFreeze(Timer& timer);
    void startFreezeCycle();

    Number calcFreezeEffect();
    Number calcSpeedEffect();
    Number calcCost();

    void updateImg();

    bool mActive = false, mCanAfford = false;

    TimerObservable::SubscriptionPtr mCostTimerSub, mFreezeDelaySub,
        mFreezeTimerSub;
    UpgradeList::SubscriptionPtr mEffectDisplay, mActiveUp, mFreezeUp, mSpeedUp;
    ParameterSystem::ParameterSubscriptionWPtr mSpeedEffectSub;

    TogglePtr mActiveToggle;

    ProgressBar mFreezePb;
};

#endif
