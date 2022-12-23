#ifndef BOT_H
#define BOT_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <Systems/IconSystem.h>
#include <Systems/TimeSystem.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Wizards/Definitions/RobotWizardDefs.h>
#include <Wizards/WizardIds.h>

#include <memory>
#include <random>

class UpgradeBot : public Component {
   public:
    UpgradeBot();

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onUpdate(Time dt);

    std::mt19937 gen = std::mt19937(rand());
    std::uniform_real_distribution<float> rDist;

    RenderAnimation mImg;
    UIComponentPtr mPos;
    SDL_FPoint mV{0, 0};
    float mTheta = 0;

    RenderObservable::SubscriptionPtr mRenderSub;
    UpdateObservable::SubscriptionPtr mUpdateSub;
    TimeSystem::TimerObservable::SubscriptionPtr mAnimTimerSub;
};

typedef std::unique_ptr<UpgradeBot> UpgradeBotPtr;

#endif
