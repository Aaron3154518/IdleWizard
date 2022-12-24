#ifndef BOT_H
#define BOT_H

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

class Bot : public Component {
   public:
    Bot();

    void move(float dx, float dy);
    Rect getPos() const;

   private:
    void init();

    void onRender(SDL_Renderer* r);

    std::mt19937 gen = std::mt19937(rand());
    std::uniform_real_distribution<float> rDist;

    RenderAnimation mImg;
    UIComponentPtr mPos;

    RenderObservable::SubscriptionPtr mRenderSub;
    TimeSystem::TimerObservable::SubscriptionPtr mAnimTimerSub;
};

typedef std::unique_ptr<Bot> BotPtr;

namespace BotAi {
struct HoverData {
    WizardId target;
    SDL_FPoint v{0, 0};
    float theta = 0;
};
void hover(Bot& bot, HoverData& data, Time dt);

struct BeelineData {
    WizardId target;
};
bool beeline(Bot& bot, BeelineData& data, Time dt);

struct DrainData {
    ParameterSystem::BaseValuePtr source;
    Number goal, rate_per_s, amnt;
};
bool drain(DrainData& data, Time dt);
}  // namespace BotAi

class UpgradeBot : public Component {
   public:
   private:
    void init();

    void onUpdate(Time dt);

    enum AiMode {
        Hover = 0,
        Drain,
    };
    AiMode mAiMode = AiMode::Hover;

    BotPtr mBot;
    BotAi::HoverData mHoverAiData;
    BotAi::BeelineData mBeelineAiData;
    BotAi::DrainData mDrainAiData;

    TimeSystem::UpdateObservable::SubscriptionPtr mUpdateSub;
    TimeSystem::TimerObservable::SubscriptionPtr mTimerSub;
};

typedef std::unique_ptr<UpgradeBot> UpgradeBotPtr;

#endif
