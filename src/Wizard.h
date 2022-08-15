#ifndef WIZARD_H
#define WIZARD_H

#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/DragService.h>
#include <ServiceSystem/EventServices/ResizeService.h>
#include <ServiceSystem/ServiceSystem.h>
#include <ServiceSystem/UpdateServices/TimerService.h>

#include <memory>
#include <vector>

#include "Fireball.h"
#include "TimeSystem.h"
#include "Upgrade.h"
#include "WizardBase.h"
#include "WizardData.h"
#include "WizardIds.h"
#include "WizardTypes.h"

class Wizard : public WizardBase {
   public:
    Wizard();

    const static std::string POWER_UP_IMG, SPEED_UP_IMG, MULTI_UP_IMG,
        POWER_BKGRND, FIREBALL_IMG;

   private:
    void init();

    void onRender(SDL_Renderer* r);
    bool onTimer();
    void onHit(WizardId src, const Number& val);

    void calcPower();
    void calcSpeed();
    void calcTimer();

    std::unique_ptr<Fireball>& shootFireball();

    RenderData mPowBkgrnd;

    TimerObservable::SubscriptionPtr mFireballTimerSub;
    TimeSystem::TimerObservable::SubscriptionPtr mPowWizTimerSub;
    TargetObservable::SubscriptionPtr mTargetSub;
    UpgradeList::SubscriptionPtr mPowerDisplay, mTargetUp, mPowerUp, mMultiUp;

    WizardId mTarget = CRYSTAL;

    std::vector<std::unique_ptr<Fireball>> mFireballs;
};

#endif