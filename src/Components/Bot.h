#ifndef BOT_H
#define BOT_H

#include <Components/UpgradeList.h>
#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <Systems/IconSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/TargetSystem.h>
#include <Systems/TimeSystem.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Wizards/Definitions/RobotWizardDefs.h>
#include <Wizards/WizardIds.h>

#include <memory>
#include <random>

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

// Forward declaration
class PowerFireballData;

class SynergyBot : public Component {
   public:
    typedef TargetSystem::TargetObservable<WizardId, const PowerFireballData&>
        HitObservableBase;
    class HitObservable : public HitObservableBase {
       public:
        class FbObservable
            : public Observable<WizardId, PowerFireballData(), UIComponentPtr> {
            friend class HitObservable;
        };
        typedef FbObservable::SubscriptionPtr FbSubscriptionPtr;

        enum { ID = 0, ON_HIT, POS };

        FbSubscriptionPtr subscribeFb(WizardId id,
                                      std::function<PowerFireballData()> onHit,
                                      UIComponentPtr pos);

        void next(WizardId id, Rect pos);

        std::vector<UIComponentCPtr> getFbPosList(WizardId id);

       private:
        using HitObservableBase::next;

        FbObservable mFbObservable;
    };

    SynergyBot(WizardId id);

    void setPos(float x, float y);

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onUpdate(Time dt);
    void onFbHit(const PowerFireballData& data);

    UIComponentPtr mPos;
    RenderAnimation mImg;

    const WizardId mTarget;
    BotAi::HoverData mHoverData;
    BotAi::BeelineData mBeelineData;

    RenderObservable::SubscriptionPtr mRenderSub;
    TimeSystem::UpdateObservable::SubscriptionPtr mUpdateSub;
    TimeSystem::TimerObservable::SubscriptionPtr mAnimTimerSub;
    HitObservable::IdSubscriptionPtr mFbHitSub;
};

typedef std::unique_ptr<SynergyBot> SynergyBotPtr;

typedef TargetSystem::TargetDataObservable<WizardId, Rect>
    SynergyBotPosObservable;

std::shared_ptr<SynergyBotPosObservable> GetSynergyBotPosObservable();

std::shared_ptr<SynergyBot::HitObservable> GetSynergyBotHitObservable();

class SynergyBotService
    : public Service<SynergyBotPosObservable, SynergyBot::HitObservable> {};

#endif