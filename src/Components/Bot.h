#ifndef BOT_H
#define BOT_H

#include <Components/UpgradeList.h>
#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <Systems/IconSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/TimeSystem.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Wizards/Definitions/RobotWizardDefs.h>
#include <Wizards/WizardIds.h>

#include <memory>
#include <random>

namespace BotAi {
struct HoverData {
    WizardId target;
    SDL_FPoint v{0, 0};
    float theta = 0, tilt = 0;
    float thetaSpd = M_PI / 3, thetaRadSpd = 7 / 11, thetaSpdSpd = 7 / 19;
    float baseRad = 100, deltaRad = 40, baseSpd = 75;
};
HoverData randomHover();
void hover(Rect& pos, HoverData& data, Time dt);

struct BeelineData {
    WizardId target;
    float tilt = 0;
};
bool beeline(Rect& pos, BeelineData& data, Time dt);
}  // namespace BotAi

class UpgradeBot : public Component {
   public:
    UpgradeBot();

    struct Arrow {
        Rect rect;
        int timer;
    };

    void setPos(float x, float y);

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onUpdate(Time dt);
    bool onUpgradeTimer(Timer& t);

    void checkUpgrades();

    void addArrow(WizardId target, int cnt);

    enum AiMode {
        HoverRobot = 0,
        HoverCrystal,
        Drain,
        Upgrade,
        Waiting,
        Paused,
    };
    AiMode mAiMode = AiMode::HoverRobot;

    UIComponentPtr mPos;
    RenderAnimation mImg;
    ProgressBar mPBar;
    std::vector<Arrow> mArrows;
    RenderData mArrowImg;

    std::mt19937 gen = std::mt19937(rand());
    std::uniform_real_distribution<float> rDist;

    BotAi::HoverData mHoverData, mHoverCrystalData;
    BotAi::BeelineData mBeelineData;

    ParameterSystem::BaseValue mSource = UpgradeDefaults::CRYSTAL_MAGIC;
    Number mAmnt, mCap = 1000, mRate = 10;
    int mNextTargetIdx = 0;

    RenderObservable::SubscriptionPtr mRenderSub;
    TimeSystem::UpdateObservable::SubscriptionPtr mUpdateSub;
    TimeSystem::TimerObservable::SubscriptionPtr mAnimTimerSub, mUpTimerSub,
        mPauseTimerSub;
};

typedef std::unique_ptr<UpgradeBot> UpgradeBotPtr;

class SynergyBot : public Component {
   public:
    SynergyBot(WizardId id);

    void setPos(float x, float y);

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onUpdate(Time dt);

    enum AiMode {
        Hover = 0,
    };
    AiMode mAiMode = AiMode::Hover;

    UIComponentPtr mPos;
    RenderAnimation mImg;

    BotAi::HoverData mHoverData;
    BotAi::BeelineData mBeelineData;

    RenderObservable::SubscriptionPtr mRenderSub;
    TimeSystem::UpdateObservable::SubscriptionPtr mUpdateSub;
    TimeSystem::TimerObservable::SubscriptionPtr mAnimTimerSub;
};

typedef std::unique_ptr<SynergyBot> SynergyBotPtr;

#endif