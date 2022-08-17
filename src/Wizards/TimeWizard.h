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
#include <Systems/ParameterSystem.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>
#include <Wizards/WizardTypes.h>

#include <memory>

class TimeWizard : public WizardBase {
   public:
    TimeWizard();

    const static std::string ACTIVE_IMG, FREEZE_IMG, FREEZE_UP_IMG;

   private:
    void init();

    bool onCostTimer(Timer& timer);
    void onRender(SDL_Renderer*);
    void onHide(WizardId id, bool hide);
    bool startFreeze(Timer& timer);
    bool endFreeze(Timer& timer);
    void startFreezeCycle();

    void calcSpeedEffect();
    void calcCost();

    void updateImg();

    bool mActive = false, mCanAfford = false;

    TimerObservable::SubscriptionPtr mCostTimerSub, mFreezeDelaySub,
        mFreezeTimerSub;
    UpgradeList::SubscriptionPtr mEffectDisplay, mActiveUp, mSpeedUp;

    ProgressBar mFreezePb;
};

#endif
