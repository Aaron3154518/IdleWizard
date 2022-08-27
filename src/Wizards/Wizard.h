#ifndef WIZARD_H
#define WIZARD_H

#include <Components/Fireballs/PowerWizFireball.h>
#include <Components/Fireballs/WizardFireball.h>
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
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Systems/TimeSystem.h>
#include <Systems/WizardSystem.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>

#include <cmath>
#include <limits>
#include <memory>
#include <vector>

class Wizard : public WizardBase {
   public:
    Wizard();

    const static unsigned int MSPF, NUM_FRAMES, POW_BK_MSPF, POW_BK_NUM_FRAMES;
    const static std::string IMG, POWER_UP_IMG, SPEED_UP_IMG, MULTI_UP_IMG,
        POWER_BKGRND, FIREBALL_IMG, FIREBALL_BUFFED_IMG, CRIT_UP_IMG;

    const static std::vector<WizardId> TARGETS;

    static void setDefaults();

   private:
    void init();
    void setSubscriptions();
    void setUpgrades();
    void setParamTriggers();

    void onRender(SDL_Renderer* r);
    void onHide(WizardId id, bool hide);
    void onReset(WizardSystem::ResetTier tier);
    bool onTimer(Timer& timer);
    void onPowFireballHit(const PowerWizFireball& fireball);
    bool onPowWizTimer(Timer& timer);
    void onPowWizTimerUpdate(Time dt, Timer& timer);
    void onFreeze(TimeSystem::FreezeType type);
    void onUnfreeze(TimeSystem::FreezeType type);

    Number calcPower();
    Number calcSpeed();
    void calcTimer();
    Number calcCrit();
    Number calcCritSpread();

    WizardFireball::Data newFireballData();

    void shootFireball();
    void shootFireball(SDL_FPoint target);

    void setPos(float x, float y);

    RenderData mPowBkgrnd;

    TimerObservable::SubscriptionPtr mFireballTimerSub;
    TimeSystem::TimerObservable::SubscriptionPtr mPowWizTimerSub, mAnimTimerSub,
        mPowBkAnimTimerSub;
    PowerWizFireball::HitObservable::IdSubscriptionPtr mPowFireballHitSub;
    TimeSystem::FreezeObservable::SubscriptionPtr mFreezeSub;
    UpgradeList::SubscriptionPtr mPowerDisplay, mTargetUp, mPowerUp, mMultiUp,
        mCritUp;

    // Pairs of effect, duration
    std::vector<std::pair<Number, Number>> mPowWizBoosts;

    WizardId mTarget = CRYSTAL;

    WizardFireballPtr mFreezeFireball;
    std::vector<WizardFireballPtr> mFireballs;
};

#endif