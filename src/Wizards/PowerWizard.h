#ifndef POWER_WIZARD_H
#define POWER_WIZARD_H

#include <Components/Fireball.h>
#include <Components/Upgrade.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>
#include <Wizards/WizardTypes.h>

#include <memory>
#include <vector>

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
