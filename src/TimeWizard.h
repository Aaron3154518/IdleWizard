#ifndef TIME_WIZARD_H
#define TIME_WIZARD_H

#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <ServiceSystem/Lockable.h>
#include <ServiceSystem/UpdateServices/TimerService.h>

#include <memory>

#include "Upgrade.h"
#include "WizardBase.h"
#include "WizardData.h"
#include "WizardIds.h"
#include "WizardTypes.h"

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

    Lock mFreezeLock;

    ProgressBar mFreezePb;
};

#endif
