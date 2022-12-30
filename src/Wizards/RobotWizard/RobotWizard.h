#ifndef ROBOT_WIZARD_H
#define ROBOT_WIZARD_H

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
#include <Wizards/Crystal/CrystalConstants.h>
#include <Wizards/Crystal/CrystalParameters.h>
#include <Wizards/PowerWizard/PowerFireball.h>
#include <Wizards/RobotWizard/Bot.h>
#include <Wizards/RobotWizard/RobotWizardConstants.h>
#include <Wizards/RobotWizard/RobotWizardParameters.h>
#include <Wizards/TimeWizard/TimeWizardConstants.h>
#include <Wizards/TimeWizard/TimeWizardParameters.h>
#include <Wizards/Wizard/WizardConstants.h>
#include <Wizards/WizardIds.h>

#include <memory>
#include <queue>
#include <unordered_map>

namespace RobotWizard {
class RobotWizard : public WizardBase {
   public:
    struct Portals {
       public:
        Portals();

        void start(const Rect& r);

       private:
        void setActive(bool active);

        RenderAnimation mPortalTopImg, mPortalBotImg;
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
    void onHide(bool hide);

    void showUpgrades();

    void upgradeTarget();

    SDL_FPoint mTargetPos;

    RenderAnimation mTpImg;
    std::unordered_map<WizardId, Portals> mPortals;

    std::unordered_map<WizardId, PowerWizard::FireballData> mStoredFireballs;
    PowerWizard::RobotFireballListPtr mFireballs;
    UpgradeBotPtr mUpBot;
    std::unordered_map<WizardId, SynergyBotPtr> mSynBots;

    TimerObservable::SubscriptionPtr mAnimTimerSub, mWaitSub;
    TimeSystem::UpdateObservable::SubscriptionPtr mMoveUpdateSub;
    UpgradeList::SubscriptionPtr mWizCritUp;
    std::vector<UpgradeList::SubscriptionPtr> mUps;
};
}  // namespace RobotWizard

#endif
