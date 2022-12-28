#ifndef BOT_H
#define BOT_H

#include <Wizards/PowerWizard/PowerFireball.h>
#include <Components/UpgradeList.h>
#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <Systems/IconSystem/IconSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/TargetSystem.h>
#include <Systems/TimeSystem.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Wizards/RobotWizard/RobotWizardConstants.h>
#include <Wizards/WizardIds.h>

#include <memory>
#include <random>

namespace RobotWizard {
namespace BotAi {
struct HoverData {
    SDL_FPoint target, v{0, 0};
    float theta = 0, tilt = 0;
    float thetaSpd = M_PI / 3, thetaRadSpd = 7 / 11, thetaSpdSpd = 7 / 19;
    float baseRad = 100, deltaRad = 40, baseSpd = 75;
};
HoverData randomHover();
void hover(Rect& pos, HoverData& data, Time dt);
void hover(Rect& pos, HoverData& data, WizardId target, Time dt);

struct BeelineData {
    SDL_FPoint target;
    float tilt = 0;
};
bool beeline(Rect& pos, BeelineData& data, Time dt);
bool beeline(Rect& pos, BeelineData& data, WizardId target, Time dt);
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

    WizardId mUpTarget;
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
    void onFbHit(const PowerWizard::Fireball& fb);
    void onFbPos(const PowerWizard::FireballList& list);
    void onTimeFreeze(bool frozen);

    bool fireballFilter(const PowerWizard::Fireball& fb) const;

    std::mt19937 gen = std::mt19937(rand());
    std::uniform_real_distribution<float> rDist;

    UIComponentPtr mPos;
    RenderAnimation mImg;

    bool mChase = false;
    const WizardId mTarget;
    BotAi::HoverData mHoverData;
    BotAi::BeelineData mBeelineData;

    PowerWizard::FireballPtr mFireball;
    PowerWizard::FireballListPtr mFireballs;

    RenderObservable::SubscriptionPtr mRenderSub;
    TimeSystem::UpdateObservable::SubscriptionPtr mUpdateSub;
    TimeSystem::TimerObservable::SubscriptionPtr mAnimTimerSub;
    PowerWizard::FireballList::HitObservable::SubscriptionPtr mFbHitSub;
    PowerWizard::FireballList::PosObservable::SubscriptionPtr mFbPosSub;
    ParameterSystem::ParameterSubscriptionPtr mFreezeSub;
};

typedef std::unique_ptr<SynergyBot> SynergyBotPtr;
}  // namespace RobotWizard

#endif