#ifndef ROBOT_WIZARD_H
#define ROBOT_WIZARD_H

#include <Components/Upgrade.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Systems/WizardSystem.h>
#include <Wizards/Definitions/RobotWizardDefs.h>
#include <Wizards/Money.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>

#include <memory>

class RobotWizard : public WizardBase {
   public:
    RobotWizard();

   private:
    void init();
    void setSubscriptions();
    void setUpgrades();
    void setParamTriggers();

    void onRender(SDL_Renderer* r);
    bool onUpTimer(Timer& t);

    int mTargetIdx = 0;

    TimerObservable::SubscriptionPtr mAnimTimerSub;
    TimeSystem::TimerObservable::SubscriptionPtr mUpTimerSub;
    UpgradeList::SubscriptionPtr mTODO;
};

#endif
