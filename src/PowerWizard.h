#ifndef POWER_WIZARD_H
#define POWER_WIZARD_H

#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/UpdateServices/TimerService.h>

#include <memory>
#include <vector>

#include "Fireball.h"
#include "Upgrade.h"
#include "WizardBase.h"
#include "WizardData.h"
#include "WizardIds.h"
#include "WizardTypes.h"

class PowerWizard : public WizardBase {
   public:
    PowerWizard();

    const static std::string FIREBALL_IMG;

   private:
    void init();

    void onRender(SDL_Renderer* r);
    bool onTimer();

    void calcPower();
    void calcSpeed();
    void calcTimer();
    void calcFireRingEffect();

    std::unique_ptr<Fireball>& shootFireball();

    TimerObservable::SubscriptionPtr mFireballTimerSub;
    UpgradeList::SubscriptionPtr mPowerDisplay;

    std::vector<std::unique_ptr<Fireball>> mFireballs;
};

#endif
