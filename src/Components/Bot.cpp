#include "Bot.h"

// BotAi
namespace BotAi {
void hover(Rect& pos, HoverData& data, Time dt) {
    float s = dt.s();
    Rect tRect = WizardSystem::GetWizardPosObservable()->get(data.target);

    // Oscillate speed and radius
    float rad = 100 + 40 * (sinf(data.theta * 7 / 11) + .25);
    float maxSpd = 75 * (sinf(data.theta * 7 / 19) + 1.5);

    float x = pos.cX(), y = pos.cY();
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
        maxSpd = 200;
    }

    // Smooth speed transition
    float a = 1.f / 30;
    data.v.x = data.v.x * (1 - a) + (dx_t * maxSpd / mag_t) * a;
    data.v.y = data.v.y * (1 - a) + (dy_t * maxSpd / mag_t) * a;
    pos.move(data.v.x * s, data.v.y * s);
}

bool beeline(Rect& pos, BeelineData& data, Time dt) {
    float s = dt.s();
    Rect tRect = WizardSystem::GetWizardPosObservable()->get(data.target);

    float x = pos.cX(), y = pos.cY();
    float tX = tRect.cX(), tY = tRect.cY();
    float dx = tX - x, dy = tY - y;
    float mag = sqrtf(dx * dx + dy * dy);

    if (mag == 0) {
        return true;
    }

    float mX = fabsf(dx) * s * 150 / mag;
    float mY = fabsf(dy) * s * 150 / mag;
    pos.move(copysignf(fminf(fabs(dx), mX), dx),
             copysignf(fminf(fabs(dy), mY), dy));

    return false;
}
}  // namespace BotAi

// UpgradeBot
UpgradeBot::UpgradeBot()
    : mPos(std::make_shared<UIComponent>(Rect(0, 0, 50, 50), Elevation::BOTS)) {
    mCap = Number(1, 15);
    mRate = Number(3, 14);

    mHoverData.target = ROBOT_WIZARD;
    mHoverCrystalData.target = CRYSTAL;
    mBeelineData.target = CRYSTAL;
}

void UpgradeBot::init() {
    mImg.set(RobotWizardDefs::IMG());
    Rect r = WizardSystem::GetWizardPosObservable()->get(mHoverData.target);
    mPos->rect.setPos(r.cX(), r.cY(), Rect::Align::CENTER);
    mPBar.set(RED, BLACK);

    mArrowImg.set("res/upgrades/arrow.png");

    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [this](SDL_Renderer* r) { onRender(r); }, mPos);
    mUpdateSub = TimeSystem::GetUpdateObservable()->subscribe(
        [this](Time dt) { onUpdate(dt); });
    mUpTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) { return onUpgradeTimer(t); }, 500);
    mAnimTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            mImg->nextFrame();
            return true;
        },
        RobotWizardDefs::IMG().frame_ms);
    mPauseTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            switch (mAiMode) {
                case AiMode::Paused:
                    checkUpgrades();
                    if (mAiMode == AiMode::Upgrade) {
                        break;
                    }
                    mUpTimerSub->setActive(true);
                case AiMode::Waiting:
                    mAiMode = AiMode::HoverRobot;
                    break;
                default:
                    break;
            };
            mPauseTimerSub->setActive(false);
            return true;
        },
        1000);
    mPauseTimerSub->setActive(false);
}

void UpgradeBot::onRender(SDL_Renderer* r) {
    TextureBuilder tex;

    mImg.setDest(mPos->rect);
    tex.draw(mImg);

    Rect pbRect(mPos->rect.x(), mPos->rect.y2(), mPos->rect.w(),
                mPos->rect.w() / 6);
    mPBar.set((mAmnt / mCap).toFloat()).set(pbRect);
    tex.draw(mPBar);

    for (auto& arrow : mArrows) {
        mArrowImg.setDest(arrow.rect);
        tex.draw(mArrowImg);
    }
}

void UpgradeBot::onUpdate(Time dt) {
    for (auto it = mArrows.begin(); it != mArrows.end(); ++it) {
        it->timer -= dt.ms();
        if (it->timer >= 200) {
            it->rect.move(0, -dt.s() * 150);
        }
        if (it->timer <= 0) {
            it = mArrows.erase(it);
            if (it == mArrows.end()) {
                break;
            }
        }
    }

    switch (mAiMode) {
        case AiMode::HoverRobot:  // Hover around robot when full
            BotAi::hover(mPos->rect, mHoverData, dt);
            if (mAmnt < mCap) {
                mAiMode = AiMode::HoverCrystal;
            }
            break;
        case AiMode::HoverCrystal:  // Hover around crystal when not full
            BotAi::hover(mPos->rect, mHoverCrystalData, dt);
            if (mSource.get() > 0) {
                mBeelineData.target = CRYSTAL;
                mAiMode = AiMode::Drain;
            }
            break;
        case AiMode::Drain:  // Drain at crystal when magic available
            if (BotAi::beeline(mPos->rect, mBeelineData, dt)) {
                Number avail = mSource.get();

                if (avail == 0) {
                    mAiMode = AiMode::Waiting;
                    mPauseTimerSub->get<TimeSystem::TimerObservable::DATA>()
                        .setLength(2000, false);
                    mPauseTimerSub->setActive(true);
                    break;
                }

                Number gain = min(avail, min(mRate * dt.s(), mCap - mAmnt));
                mSource.set(mSource.get() - gain);
                mAmnt += gain;
                if (mAmnt == mCap) {
                    mAiMode = AiMode::HoverRobot;
                }
            }
            break;
        case AiMode::Upgrade:  // Perform upgrade when available
            if (BotAi::beeline(mPos->rect, mBeelineData, dt)) {
                auto upgrades = GetWizardUpgrades(mBeelineData.target);
                auto upRes = upgrades->upgradeAll(mSource, mAmnt);
                mAmnt -= upRes.moneySpent;
                addArrow(mBeelineData.target, upRes.levelCnt);
                mAiMode = AiMode::Paused;
                mPauseTimerSub->get<TimeSystem::TimerObservable::DATA>()
                    .setLength(500, false);
                mPauseTimerSub->setActive(true);
            }
            break;
        case AiMode::Waiting:  // Waitng to drain more
            if (mSource.get() > 0) {
                mAiMode = AiMode::Drain;
            }
            break;
        case AiMode::Paused:  // Pausing after upgrade
            break;
    }
}

bool UpgradeBot::onUpgradeTimer(Timer& t) {
    checkUpgrades();
    return true;
}

void UpgradeBot::checkUpgrades() {
    int currIdx = mNextTargetIdx;
    do {
        WizardId target = RobotWizardDefs::TARGETS.at(mNextTargetIdx);
        mNextTargetIdx = (mNextTargetIdx + 1) % RobotWizardDefs::TARGETS.size();
        if (!WizardSystem::Hidden(target) &&
            GetWizardUpgrades(target)->canBuyOne(mSource, mAmnt)) {
            mAiMode = AiMode::Upgrade;
            mBeelineData.target = target;
            mUpTimerSub->setActive(false);
            break;
        }
    } while (mNextTargetIdx != currIdx);
}

void UpgradeBot::addArrow(WizardId target, int cnt) {
    for (int i = 0; i < cnt; i++) {
        Rect tRect = WizardSystem::GetWizardPos(target);
        mArrows.push_back(
            Arrow{Rect(0, 0, 30, 40), (int)(rDist(gen) * 250) + 500});
        mArrows.back().rect.setPos(tRect.cX() + (rDist(gen) - .5) * 50,
                                   tRect.cY() + rDist(gen) * 30,
                                   Rect::Align::CENTER);
    }
}

void UpgradeBot::setPos(float x, float y) {
    mPos->rect.setPos(x, y, Rect::Align::CENTER);
}
