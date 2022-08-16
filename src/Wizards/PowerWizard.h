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
    void onHide(WizardId id, bool hide);
    bool onTimer(Timer& timer);
    void onFreeze(TimeSystem::FreezeType type);
    void onUnfreeze(TimeSystem::FreezeType type);

    void calcPower();
    void calcSpeed();
    void calcTimer();
    void calcFireRingEffect();

    void shootFireball();
    void shootFireball(SDL_FPoint target);

    void setPos(float x, float y);

    TimerObservable::SubscriptionPtr mFireballTimerSub;
    TimeSystem::FreezeObservable::SubscriptionPtr mFreezeSub;
    UpgradeList::SubscriptionPtr mPowerDisplay;

    std::vector<FireballPtr> mFireballs;
    int mFireballFreezeCnt;
};

#endif
