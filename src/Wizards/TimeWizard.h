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

    void onUpdate(Time dt);
    void onRender(SDL_Renderer*);
    bool startFreeze();
    bool endFreeze();

    void calcCost();

    void updateImg();

    bool mActive = false, mCanAfford = false;

    UpdateObservable::SubscriptionPtr mUpdateSub;
    TimerObservable::SubscriptionPtr mFreezeDelaySub, mFreezeTimerSub;
    UpgradeList::SubscriptionPtr mEffectDisplay, mActiveUp;

    ProgressBar mFreezePb;
};

#endif
