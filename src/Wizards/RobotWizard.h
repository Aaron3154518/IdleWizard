#ifndef ROBOT_WIZARD_H
#define ROBOT_WIZARD_H

#include <Components/Fireballs/PowerWizFireball.h>
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
    bool onUpgradeTimer(Timer& t);
    void onRender(SDL_Renderer* r);
    void onResize(ResizeData data);
    void onPowFireballHit(const PowerWizFireball& fireball);

    void upgradeTarget();

    WizardId mTarget = WizardId::size;
    int mTargetIdx = 0;
    SDL_FPoint mTargetPos;

    RenderData mTpImg;
    std::unordered_map<WizardId, Portals> mPortals;

    std::unordered_map<WizardId, PowerWizFireball::Data> mStoredFireballs;
    std::vector<PowerWizFireballPtr> mFireballs;

    TimerObservable::SubscriptionPtr mAnimTimerSub, mWaitSub;
    PowerWizFireball::HitObservable::IdSubscriptionPtr mPowFireballHitSub;
    TimeSystem::UpdateObservable::SubscriptionPtr mMoveUpdateSub;
    TimeSystem::TimerObservable::SubscriptionPtr mUpTimerSub;
    UpgradeList::SubscriptionPtr mTODO;
};

#endif
