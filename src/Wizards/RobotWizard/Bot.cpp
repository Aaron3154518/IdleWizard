#include "Bot.h"

#include <Wizards/PowerWizard/PowerFireball.h>

namespace RobotWizard {
// BotAi
namespace BotAi {
HoverData randomHover() {
    static std::mt19937 gen = std::mt19937(rand());
    static std::uniform_real_distribution<float> rDist;

    HoverData data;
    data.theta = rDist(gen) * 2 * M_PI;
    data.thetaSpd *= (rDist(gen) / 2) + .75;
    data.thetaRadSpd *= rDist(gen) + .5;
    data.thetaSpdSpd *= rDist(gen) + .5;
    data.baseRad *= (rDist(gen) / 2) + .75;
    data.deltaRad *= (rDist(gen) / 4) + .875;
    data.baseSpd *= (rDist(gen) / 2) + .75;

    return data;
}

void hover(Rect& pos, HoverData& data, WizardId target, Time dt) {
    data.target = WizardSystem::GetWizardPosObservable()->get(target).getPos(
        Rect::Align::CENTER);
    hover(pos, data, dt);
}
void hover(Rect& pos, HoverData& data, Time dt) {
    float s = dt.s();

    // Oscillate speed and radius
    float rad = data.baseRad +
                data.deltaRad * (sinf(data.theta * data.thetaRadSpd) + .25);
    float maxSpd = data.baseSpd * (sinf(data.theta * data.thetaSpdSpd) + 1.5);

    float x = pos.cX(), y = pos.cY();
    float x_t = rad * cosf(data.theta) + data.target.x,
          y_t = rad / 3 * sinf(data.theta) + data.target.y;

    float dx = data.target.x - x, dy = data.target.y - y;
    float mag = sqrtf(dx * dx + dy * dy);

    if (mag == 0) {
        mag = 1;
    }

    float dx_t = x_t - x, dy_t = y_t - y;
    float mag_t = sqrtf(dx_t * dx_t + dy_t * dy_t);

    if (mag < rad * 1.1) {
        data.theta = fmodf(data.theta + s * data.thetaSpd, 2 * M_PI);
    } else {
        maxSpd = 3 * data.baseSpd;
    }

    // Smooth speed transition
    float a = 1.f / 30;
    data.v.x = data.v.x * (1 - a) + (dx_t * maxSpd / mag_t) * a;
    data.v.y = data.v.y * (1 - a) + (dy_t * maxSpd / mag_t) * a;
    pos.move(data.v.x * s, data.v.y * s);

    // Sprite tilt
    float tilt =
        copysignf(powf(1 / (1 + expf(-fabsf(data.v.x) / 40 + 1)), 2), data.v.x);
    if (fabsf(tilt) < .1f) {
        tilt = 0;
    }
    tilt *= M_PI / 4;
    a = 1.f / 10;
    data.tilt = data.tilt * (1 - a) + tilt * a;
}

bool beeline(Rect& pos, BeelineData& data, WizardId target, Time dt) {
    data.target = WizardSystem::GetWizardPosObservable()->get(target).getPos(
        Rect::Align::CENTER);
    return beeline(pos, data, dt);
}
bool beeline(Rect& pos, BeelineData& data, Time dt) {
    float s = dt.s();

    float x = pos.cX(), y = pos.cY();
    float dx = data.target.x - x, dy = data.target.y - y;
    float mag = sqrtf(dx * dx + dy * dy);
    float vX = mag == 0 ? 0 : dx / mag;

    // Sprite tilt
    data.tilt = copysignf(powf(1 / (1 + expf(-fabsf(vX) * 3 + 1)), 2), vX);
    if (fabsf(data.tilt) < .1f) {
        data.tilt = 0;
    }
    data.tilt *= M_PI / 4;

    if (mag < 1e-5) {
        return true;
    }

    float frac = fminf(s * data.speed / mag, 1);
    pos.move(dx * frac, dy * frac);

    return false;
}
}  // namespace BotAi

// UpgradeBot
UpgradeBot::UpgradeBot()
    : mHoverData(BotAi::randomHover()),
      mPos(std::make_shared<UIComponent>(Rect(0, 0, 60, 30), Elevation::BOTS)) {
    mPos->mouse = false;
}

void UpgradeBot::init() {
    mImg.set(RobotWizard::Constants::BOT_IMG());
    Rect r = WizardSystem::GetWizardPosObservable()->get(ROBOT_WIZARD);
    setPos(r.cX(), r.cY());
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
        RobotWizard::Constants::BOT_IMG());
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

    RobotWizard::Params roboParams;
    Number cap = roboParams[RobotWizard::Param::UpBotCap].get();
    if (cap == 0) {
        mPBar.set(1);
    } else {
        mPBar.set((mAmnt / cap).toFloat());
    }
    if (mPBar.get().perc < 1) {
        mPBar.set(Rect(mPos->rect.x(), mPos->rect.y2(), mPos->rect.w(),
                       mPos->rect.h() / 4));
        tex.draw(mPBar);
    }

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

    RobotWizard::Params roboParams;
    Number cap = roboParams[RobotWizard::Param::UpBotCap].get(),
           rate = roboParams[RobotWizard::Param::UpBotRate].get();

    switch (mAiMode) {
        case AiMode::HoverRobot:  // Hover around robot when full
            BotAi::hover(mPos->rect, mHoverData, ROBOT_WIZARD, dt);
            mImg.setRotationRad(mHoverData.tilt);
            if (mAmnt < cap) {
                mAiMode = AiMode::HoverCrystal;
            }
            break;
        case AiMode::HoverCrystal:  // Hover around crystal when not full
            BotAi::hover(mPos->rect, mHoverCrystalData, CRYSTAL, dt);
            mImg.setRotationRad(mHoverCrystalData.tilt);
            if (mSource.get() > 0) {
                mAiMode = AiMode::Drain;
            }
            break;
        case AiMode::Drain:  // Drain at crystal when magic available
            if (BotAi::beeline(mPos->rect, mBeelineData, CRYSTAL, dt)) {
                Number avail = mSource.get();

                if (avail == 0) {
                    mAiMode = AiMode::Waiting;
                    mPauseTimerSub->get<TimeSystem::TimerObservable::DATA>()
                        .setLength(2000, false);
                    mPauseTimerSub->setActive(true);
                    break;
                }

                Number gain = Number::min(avail, Number::min(rate * dt.s(), cap - mAmnt));
                mSource.set(mSource.get() - gain);
                mAmnt += gain;
                if (mAmnt == cap) {
                    mAiMode = AiMode::HoverRobot;
                }
            }
            mImg.setRotationRad(mBeelineData.tilt);
            break;
        case AiMode::Upgrade:  // Perform upgrade when available
            if (BotAi::beeline(mPos->rect, mBeelineData, mUpTarget, dt)) {
                auto upgrades = GetWizardUpgrades(mUpTarget);
                auto upRes = upgrades->upgradeAll(mSource, mAmnt);
                mAmnt -= upRes.moneySpent;
                addArrow(mUpTarget, upRes.levelCnt);
                mAiMode = AiMode::Paused;
                mPauseTimerSub->get<TimeSystem::TimerObservable::DATA>()
                    .setLength(500, false);
                mPauseTimerSub->setActive(true);
            }
            mImg.setRotationRad(mBeelineData.tilt);
            break;
        case AiMode::Waiting:  // Waiting to drain more
            if (mSource.get() > 0) {
                mAiMode = AiMode::Drain;
            }
        case AiMode::Paused:  // Pausing after upgrade
            mImg.setRotationRad(0);
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
        WizardId target = RobotWizard::Constants::UP_TARGETS.at(mNextTargetIdx);
        mNextTargetIdx =
            (mNextTargetIdx + 1) % RobotWizard::Constants::UP_TARGETS.size();
        if (!WizardSystem::Hidden(target) &&
            GetWizardUpgrades(target)->canBuyOne(mSource, mAmnt)) {
            mAiMode = AiMode::Upgrade;
            mUpTarget = target;
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

// SynergyBot
SynergyBot::SynergyBot(WizardId id)
    : mTarget(id),
      mHoverData(BotAi::randomHover()),
      mPos(std::make_shared<UIComponent>(Rect(0, 0, 0, 0), Elevation::BOTS)) {
    mPos->mouse = false;
}

void SynergyBot::init() {
    mFireballs = ComponentFactory<PowerWizard::RobotFireballList>::New();

    mImg.set(RobotWizard::Constants::BOT_FLOAT_IMG(mTarget));

    Rect r = WizardSystem::GetWizardPosObservable()->get(mTarget);
    mPos->rect = mImg->getMinRect(45, 45);
    mPos->rect.setPos(r.cX(), r.cY(), Rect::Align::CENTER);

    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [this](SDL_Renderer* r) { onRender(r); }, mPos);
    mUpdateSub = TimeSystem::GetUpdateObservable()->subscribe(
        [this](Time dt) { onUpdate(dt); });
    mFbHitSub = PowerWizard::FireballList::GetHitObservable()->subscribe(
        [this](const PowerWizard::Fireball& fb) { onFbHit(fb); },
        [this](const PowerWizard::Fireball& fb) { return fireballFilter(fb); },
        mPos);
    mFbPosSub = PowerWizard::FireballList::GetPosObservable()->subscribe(
        [this](const PowerWizard::FireballList& list) { onFbPos(list); });

    mFreezeSub = TimeWizard::Params::get(TimeWizard::Param::Frozen)
                     .subscribe([this](bool frozen) { onTimeFreeze(frozen); });
}

void SynergyBot::onRender(SDL_Renderer* r) {
    TextureBuilder tex;

    if (mFireball) {
        mFireball->setPos(mPos->rect.cX(), mPos->rect.y2());
        mFireball->draw(tex);
    }

    mImg.setDest(mPos->rect);
    tex.draw(mImg);
}

void SynergyBot::onUpdate(Time dt) {
    if (mChase) {
        BotAi::beeline(mPos->rect, mBeelineData, dt);
        mImg.setRotationRad(mBeelineData.tilt);
    } else {  // Hover around target
        BotAi::hover(mPos->rect, mHoverData, mTarget, dt);
        mImg.setRotationRad(mHoverData.tilt);
    }
}

void SynergyBot::onFbHit(const PowerWizard::Fireball& fb) {
    auto data = fb.getData();
    data.fromBot = true;
    data.speed = PowerWizard::FireballData{}.speed * 5;
    if (mFireball) {
        auto currData = mFireball->getData();
        data.power = Number::max(data.power, currData.power);
        data.duration = Number::max(data.duration, currData.duration);
        data.sizeFactor = fmaxf(data.sizeFactor, currData.sizeFactor);
    } else {  // Slow down
        mHoverData.baseSpd = BotAi::HoverData{}.baseSpd / 2;
        mBeelineData.speed = BotAi::BeelineData{}.speed / 2;
    }
    mFireball = ComponentFactory<PowerWizard::Fireball>::New(SDL_FPoint{0, 0},
                                                             mTarget, data);
}

void SynergyBot::onFbPos(const PowerWizard::FireballList& list) {
    float minD = std::numeric_limits<float>::max();
    SDL_FPoint target;
    for (auto& fb : list) {
        if (!fireballFilter(fb)) {
            continue;
        }

        SDL_FPoint fbPos = fb.getPos().getPos(Rect::Align::CENTER);
        float dx = fbPos.x - mPos->rect.cX(), dy = fbPos.y - mPos->rect.cY();
        float d = sqrtf(dx * dx + dy * dy);
        if (d < minD) {
            minD = d;
            target = fbPos;
        }
    }

    mChase = minD <= 300;
    mBeelineData.target = target;
}

bool SynergyBot::fireballFilter(const PowerWizard::Fireball& fb) const {
    return fb.getTargetId() == mTarget && !fb.isFromBot();
}

void SynergyBot::onTimeFreeze(bool frozen) {
    // Make sure we actually have a fireball
    if (!mFireball) {
        return;
    }

    // Make sure we have a valid target
    if (frozen && mTarget != WIZARD) {
        return;
    }
    if (!frozen && mTarget != CRYSTAL && mTarget != TIME_WIZARD) {
        return;
    }

    // Speed up
    mHoverData.baseSpd = BotAi::HoverData{}.baseSpd;
    mBeelineData.speed = BotAi::BeelineData{}.speed;

    SDL_FPoint pos =
        WizardSystem::GetWizardPos(mTarget).getPos(Rect::Align::CENTER);
    mFireball->launch(pos);
    mFireballs->push_back(std::move(mFireball));
}

void SynergyBot::setPos(float x, float y) {
    mPos->rect.setPos(x, y, Rect::Align::CENTER);
}
}  // namespace RobotWizard
