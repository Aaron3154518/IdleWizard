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
#include <Systems/ParameterSystem/Parameter.h>
#include <Systems/ParameterSystem/WizardParams.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>

#include <memory>
#include <vector>

class PowerWizard : public WizardBase {
   public:
    PowerWizard();

    const static std::string FIREBALL_IMG, POWER_UP_IMG;

   private:
    void setDefaultValues();
    void setSubscriptions();
    void setUpgrades();
    void setFormulas();

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
    UpgradeList::SubscriptionPtr mPowerDisplay, mPowerUp;

    std::vector<FireballPtr> mFireballs;
    int mFireballFreezeCnt;
};

#endif
