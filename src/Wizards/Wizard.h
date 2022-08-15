#ifndef WIZARD_H
#define WIZARD_H

#include <Components/Fireball.h>
#include <Components/Upgrade.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/DragService.h>
#include <ServiceSystem/EventServices/ResizeService.h>
#include <ServiceSystem/ServiceSystem.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem.h>
#include <Systems/TimeSystem.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>
#include <Wizards/WizardTypes.h>

#include <memory>
#include <vector>

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