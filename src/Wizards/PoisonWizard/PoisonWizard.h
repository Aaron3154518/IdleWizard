#ifndef POISON_WIZARD_H
#define POISON_WIZARD_H

#include <Components/Upgrade.h>
#include <Components/WizardBase.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <Systems/IconSystem/IconSystem.h>
#include <Systems/IconSystem/MoneyIcons.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Wizards/Catalyst/CatalystConstants.h>
#include <Wizards/Crystal/CrystalConstants.h>
#include <Wizards/PoisonWizard/Glob.h>
#include <Wizards/PoisonWizard/PoisonFireball.h>
#include <Wizards/PoisonWizard/PoisonWizardConstants.h>
#include <Wizards/Wizard/WizardConstants.h>
#include <Wizards/WizardIds.h>

#include <memory>

namespace PoisonWizard {
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
    Number calcBlobCount();

    void setTargets();

    Fireball::Data newFireballData();

    void shootFireball();

    TimerObservable::SubscriptionPtr mFireballTimerSub;
    TimeSystem::TimerObservable::SubscriptionPtr mAnimTimerSub;
    UpgradeList::SubscriptionPtr mPoisonDisplay, mShardMultUp, mPoisonFbUp,
        mGlobCntUp, mCatPoisUp;
    WizardSystem::WizardEventObservable::IdSubscriptionPtr mT1ResetSub;

    std::vector<WizardId> mTargets;
    FireballListPtr mFireballs;
    std::vector<GlobPtr> mGlobs;
};
}  // namespace PoisonWizard

#endif
