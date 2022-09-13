#ifndef POISON_WIZARD_H
#define POISON_WIZARD_H

#include <Components/Fireballs/Glob.h>
#include <Components/Fireballs/PoisonFireball.h>
#include <Components/Upgrade.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <Systems/IconSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Systems/WizardSystem.h>
#include <Wizards/Definitions/CatalystDefs.h>
#include <Wizards/Definitions/PoisonWizardDefs.h>
#include <Wizards/Money.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>

#include <memory>

class PoisonWizard : public WizardBase {
   public:
    PoisonWizard();

   private:
    void init();
    void setSubscriptions();
    void setUpgrades();
    void setParamTriggers();

    void onRender(SDL_Renderer* r);
    void onHide(bool hide);
    bool onFireballTimer(Timer& t);
    void onT1Reset();

    Number calcSpeed();
    void calcTimer();

    PoisonFireball::Data newFireballData();

    void shootFireball();

    TimerObservable::SubscriptionPtr mFireballTimerSub;
    TimeSystem::TimerObservable::SubscriptionPtr mAnimTimerSub;
    UpgradeList::SubscriptionPtr mCatGainUp1, mCatGainUp2;
    WizardSystem::WizardEventObservable::IdSubscriptionPtr mT1ResetSub;

    std::vector<PoisonFireballPtr> mFireballs;
    std::vector<GlobPtr> mGlobs;
};

#endif
