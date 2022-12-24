#include "Bot.h"

// Bot
Bot::Bot()
    : mPos(std::make_shared<UIComponent>(Rect(0, 0, 50, 50), Elevation::BOTS)) {
}

void Bot::init() {
    mImg.set(RobotWizardDefs::IMG());

    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [this](SDL_Renderer* r) { onRender(r); }, mPos);

    mAnimTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            mImg->nextFrame();
            return true;
        },
        RobotWizardDefs::IMG().frame_ms);
}

void Bot::onRender(SDL_Renderer* r) {
    TextureBuilder tex;

    mImg.setDest(mPos->rect);
    tex.draw(mImg);
}

void Bot::move(float dx, float dy) { mPos->rect.move(dx, dy); }
Rect Bot::getPos() const { return mPos->rect; }

namespace BotAi {
void hover(Bot& bot, HoverData& data, Time dt) {
    float s = dt.s();
    Rect tRect = WizardSystem::GetWizardPosObservable()->get(data.target);

    // Oscillate speed and radius
    float rad = 100 + 40 * (sinf(data.theta * 7 / 11) + .25);
    float maxSpd = 75 * (sinf(data.theta * 7 / 19) + 1.5);

    Rect botRect = bot.getPos();
    float x = botRect.cX(), y = botRect.cY();
    float tX = tRect.cX(), tY = tRect.cY();
    float x_t = rad * cosf(data.theta) + tX,
          y_t = rad / 3 * sinf(data.theta) + tY;

    float dx = tX - x, dy = tY - y;
    float mag = sqrtf(dx * dx + dy * dy);

    float dx_t = x_t - x, dy_t = y_t - y;
    float mag_t = sqrtf(dx_t * dx_t + dy_t * dy_t);

    if (mag < rad * 1.1) {
        data.theta += s * M_PI / 3;
    } else {
        maxSpd = 150;
    }

    // Smooth speed transition
    data.v.x = data.v.x * (1 - s) + (dx_t * maxSpd / mag_t) * s;
    data.v.y = data.v.y * (1 - s) + (dy_t * maxSpd / mag_t) * s;
    bot.move(data.v.x * s, data.v.y * s);
}

bool beeline(Bot& bot, BeelineData& data, Time dt) {
    float s = dt.s();
    Rect tRect = WizardSystem::GetWizardPosObservable()->get(data.target);

    Rect botRect = bot.getPos();
    float x = botRect.cX(), y = botRect.cY();
    float tX = tRect.cX(), tY = tRect.cY();
    float dx = tX - x, dy = tY - y;
    float mag = sqrtf(dx * dx + dy * dy);

    if (mag == 0) {
        return true;
    }

    float mX = fabsf(dx) * s * 150 / mag;
    float mY = fabsf(dy) * s * 150 / mag;
    bot.move(copysignf(fminf(fabs(dx), mX), dx),
             copysignf(fminf(fabs(dy), mY), dy));

    return false;
}

bool drain(DrainData& data, Time dt) {
    if (!data.source) {
        return false;
    }

    Number gain = min(data.source->get(),
                      min(data.rate_per_s * dt.s(), data.goal - data.amnt));

    data.source->set(data.source->get() - gain);
    data.amnt += gain;

    return data.amnt >= data.goal;
}
}  // namespace BotAi

// UpgradeBot
void UpgradeBot::init() {
    mBot = ComponentFactory<Bot>::New();
    mHoverAiData.target = ROBOT_WIZARD;
    mBeelineAiData.target = CRYSTAL;
    mDrainAiData.source = std::make_unique<ParameterSystem::BaseValue>(
        ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic));
    mDrainAiData.goal = 100;
    mDrainAiData.rate_per_s = 50;

    mUpdateSub = TimeSystem::GetUpdateObservable()->subscribe(
        [this](Time dt) { onUpdate(dt); });

    mTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            mDrainAiData.amnt = 0;
            mAiMode = AiMode::Drain;
            mTimerSub->setActive(false);
            return true;
        },
        5000);
}

void UpgradeBot::onUpdate(Time dt) {
    switch (mAiMode) {
        case AiMode::Hover:
            BotAi::hover(*mBot, mHoverAiData, dt);
            break;
        case AiMode::Drain:
            if (BotAi::beeline(*mBot, mBeelineAiData, dt)) {
                if (BotAi::drain(mDrainAiData, dt)) {
                    mAiMode = AiMode::Hover;
                    mTimerSub->setActive(true);
                }
            }
            break;
    }
}
