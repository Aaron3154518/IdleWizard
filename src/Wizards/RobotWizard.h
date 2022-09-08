#ifndef ROBOT_WIZARD_H
#define ROBOT_WIZARD_H

#include <Components/Fireballs/PowerWizFireball.h>
#include <Components/Upgrade.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Systems/WizardSystem.h>
#include <Wizards/Definitions/CrystalDefs.h>
#include <Wizards/Definitions/RobotWizardDefs.h>
#include <Wizards/Definitions/TimeWizardDefs.h>
#include <Wizards/Definitions/WizardDefs.h>
#include <Wizards/Money.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>

#include <memory>
#include <queue>
#include <unordered_map>

class RobotWizard : public WizardBase {
   public:
    struct Portals {
       public:
        Portals();

        void start(const Rect& r);

       private:
        void setActive(bool active);

        RenderData mPortalTopImg, mPortalBotImg;
        TimerObservable::SubscriptionPtr mPortalTimerSub;
        UIComponentPtr mPortalTopPos, mPortalBotPos;
        RenderObservable::SubscriptionPtr mPortalTopRenderSub,
            mPortalBotRenderSub;
    };

    RobotWizard();

   private:
    void init();
    void setSubscriptions();
    void setUpgrades();
    void setParamTriggers();

    void onMoveUpdate(Time dt);
    void onRender(SDL_Renderer* r);
    bool onUpTimer(Timer& t);
    void onPowFireballHit(const PowerWizFireball& fireball);

    int mTargetIdx = 0;

    RenderData mTpImg;
    std::unordered_map<WizardId, Portals> mPortals;

    std::unordered_map<WizardId, PowerWizFireball::Data> mStoredFireballs;
    std::vector<PowerWizFireballPtr> mFireballs;

    TimerObservable::SubscriptionPtr mAnimTimerSub;
    PowerWizFireball::HitObservable::IdSubscriptionPtr mPowFireballHitSub;
    TimeSystem::UpdateObservable::SubscriptionPtr mMoveUpdateSub;
    TimeSystem::TimerObservable::SubscriptionPtr mUpTimerSub;
    UpgradeList::SubscriptionPtr mTODO;
};

#endif
