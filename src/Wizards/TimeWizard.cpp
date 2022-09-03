#include "TimeWizard.h"

// TimeWizard
const AnimationData TimeWizard::IMG{"res/wizards/time_wizard_ss.png", 8, 100},
    TimeWizard::FREEZE_IMG{"res/wizards/time_wizard_frozen_ss.png", 9, 100};

const std::string TimeWizard::FREEZE_UP_IMG =
    "res/upgrades/time_freeze_upgrade.png";
const std::string TimeWizard::SPEED_UP_IMG = "res/upgrades/speed_upgrade.png";

void TimeWizard::setDefaults() {
    using WizardSystem::Event;

    ParameterSystem::Params<TIME_WIZARD> params;

    params[TimeWizardParams::SpeedBaseEffect]->init(1.5);
    params[TimeWizardParams::FreezeBaseEffect]->init(1.1);
    params[TimeWizardParams::FreezeDelay]->init(100000);
    params[TimeWizardParams::FreezeDuration]->init(5000);
    params[TimeWizardParams::TimeWarpEffect]->init(1, Event::ResetT1);

    params[TimeWizardParams::SpeedUpLvl]->init(Event::ResetT1);
    params[TimeWizardParams::FBSpeedUpLvl]->init(Event::ResetT1);
    params[TimeWizardParams::FreezeUpLvl]->init(Event::ResetT1);
    params[TimeWizardParams::BoostWizSpdUpLvl]->init(Event::ResetT1);

    ParameterSystem::States states;

    states[State::TimeWizActive]->init(false, Event::ResetT1);
    states[State::TimeWizFrozen]->init(false, Event::ResetT1);
}

RenderDataWPtr TimeWizard::GetIcon() {
    static RenderDataPtr ICON;
    static TimerObservable::SubscriptionPtr ANIM_SUB;
    if (!ICON) {
        ICON = std::make_shared<RenderData>();
        ICON->set(IMG);
        ANIM_SUB =
            ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
                [](Timer& t) {
                    ICON->nextFrame();
                    return true;
                },
                Timer(IMG.frame_ms));
    }

    return ICON;
}

TimeWizard::TimeWizard() : WizardBase(TIME_WIZARD) {}

void TimeWizard::init() {
    updateImg();
    mImg.setDest(IMG_RECT);
    mPos->rect = mImg.getDest();

    mFreezePb = ProgressBar(SDL_BLENDMODE_BLEND).set(BLACK, BLACK);

    WizardBase::init();
}
void TimeWizard::setSubscriptions() {
    mCostTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            [this](Timer& t) { return onCostTimer(t); }, Timer(50));
    mAnimTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            [this](Timer& t) {
                mImg.nextFrame();
                WizardSystem::GetWizardImageObservable()->next(mId, mImg);
                if (ParameterSystem::Param(State::TimeWizFrozen).get()) {
                    t.length = mImg.getFrame() != 0
                                   ? FREEZE_IMG.frame_ms
                                   : (int)(rDist(gen) * 500) + 1000;
                }
                return true;
            },
            IMG);
    mT1ResetSub = WizardSystem::GetWizardEventObservable()->subscribe(
        [this]() { onT1Reset(); }, WizardSystem::Event::ResetT1);
    mPowFireballHitSub = PowerWizFireball::GetHitObservable()->subscribe(
        [this](const PowerWizFireball& fireball) {
            onPowFireballHit(fireball);
        },
        mId);
    attachSubToVisibility(mCostTimerSub);
}
void TimeWizard::setUpgrades() {
    ParameterSystem::Params<TIME_WIZARD> params;
    ParameterSystem::States states;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(mId);
    dUp->setDescription({"Speed multiplier"});
    dUp->setEffects({params[TimeWizardParams::FreezeDelay],
                     params[TimeWizardParams::FreezeDuration],
                     params[TimeWizardParams::FreezeEffect]},
                    {}, []() -> TextUpdateData {
                        ParameterSystem::Params<TIME_WIZARD> params;
                        std::stringstream ss;
                        ss << "Cooldown: "
                           << params[TimeWizardParams::FreezeDelay].get() / 1000
                           << "s\nDuration: "
                           << params[TimeWizardParams::FreezeDuration].get() /
                                  1000
                           << "s\nUnfreeze Effect: Power ^ "
                           << params[TimeWizardParams::FreezeEffect].get();
                        return {ss.str()};
                    });
    mEffectDisplay = mUpgrades->subscribe(dUp);

    // Active toggle
    mActiveToggle = std::make_shared<Toggle>(
        [this](unsigned int state, Toggle& tUp) {
            ParameterSystem::Param(State::TimeWizActive).set(state == 1);
        },
        2);
    mActiveToggle->setImage(WIZ_IMGS.at(mId));
    mActiveToggle->setDescription(
        {"Consume magic for a fire rate multiplier to all Wizards"});
    mActiveToggle->setEffects(
        {params[TimeWizardParams::SpeedCost],
         params[TimeWizardParams::SpeedEffect]},
        {}, []() -> TextUpdateData {
            ParameterSystem::Params<TIME_WIZARD> params;
            std::stringstream ss;
            ss << params[TimeWizardParams::SpeedEffect].get() << "x\n-"
               << (params[TimeWizardParams::SpeedCost].get() * 100) << "%{i}/s";
            return {ss.str(),
                    {Money::GetMoneyIcon(Upgrade::Defaults::CRYSTAL_MAGIC)}};
        });
    mActiveUp = mUpgrades->subscribe(mActiveToggle);

    // Speed upgrade
    UpgradePtr up =
        std::make_shared<Upgrade>(params[TimeWizardParams::SpeedUpLvl], 10);
    up->setImage(SPEED_UP_IMG);
    up->setDescription(
        {"Increase speed boost multiplier by +.05\nThis will also increase "
         "the magic cost!"});
    up->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                params[TimeWizardParams::SpeedUpCost],
                [](const Number& lvl) { return 250 ^ (lvl / 12 + 1); });
    up->setEffect(
        params[TimeWizardParams::SpeedUp],
        [](const Number& lvl) { return lvl * .05; },
        Upgrade::Defaults::AdditiveEffect);
    mSpeedUp = mUpgrades->subscribe(up);

    // Fireball Speed upgrade
    up = std::make_shared<Upgrade>(params[TimeWizardParams::FBSpeedUpLvl], 6);
    up->setImage("");
    up->setDescription(
        {"Increase fireball speed *1.075\nHigher speed gives more power"});
    up->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                params[TimeWizardParams::FBSpeedCost],
                [](const Number& lvl) { return 100 * (2.5 ^ lvl); });
    up->setEffect(
        params[TimeWizardParams::FBSpeedUp],
        [](const Number& lvl) { return 1.075 ^ lvl; },
        Upgrade::Defaults::MultiplicativeEffect);
    mFBSpeedUp = mUpgrades->subscribe(up);

    // Freeze upgrade
    up = std::make_shared<Upgrade>(params[TimeWizardParams::FreezeUpLvl], 8);
    up->setImage(FREEZE_UP_IMG);
    up->setDescription({"Multiply unfreeze boost exponent by 1.03"});
    up->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                params[TimeWizardParams::FreezeUpCost],
                [](const Number& lvl) { return 150 * (1.6 ^ lvl); });
    up->setEffect(
        params[TimeWizardParams::FreezeUp],
        [](const Number& lvl) { return 1.03 ^ lvl; },
        Upgrade::Defaults::MultiplicativeEffect);
    mFreezeUp = mUpgrades->subscribe(up);

    // Boosted wizard speed upgrade
    up = std::make_shared<Upgrade>(params[TimeWizardParams::BoostWizSpdUpLvl],
                                   8);
    up->setImage("");
    up->setDescription(
        {"Multiplies wizard fire rate by *1.1/level while boosted by power "
         "wizard"});
    up->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                params[TimeWizardParams::BoostWizSpdUpCost],
                [](const Number& lvl) { return Number(1, 3) * (2.1 ^ lvl); });
    up->setEffect(
        params[TimeWizardParams::BoostWizSpdUp],
        [](const Number& lvl) { return 1.1 ^ lvl; },
        Upgrade::Defaults::MultiplicativeEffect);
    mBoostWizSpdUp = mUpgrades->subscribe(up);
}
void TimeWizard::setParamTriggers() {
    ParameterSystem::Params<TIME_WIZARD> params;
    ParameterSystem::States states;

    mParamSubs.push_back(params[TimeWizardParams::FreezeEffect].subscribeTo(
        {params[TimeWizardParams::FreezeBaseEffect],
         params[TimeWizardParams::FreezeUp]},
        {}, [this]() { return calcFreezeEffect(); }));

    mParamSubs.push_back(params[TimeWizardParams::SpeedEffect].subscribeTo(
        {params[TimeWizardParams::SpeedBaseEffect],
         params[TimeWizardParams::SpeedUp]},
        {states[State::TimeWizActive], states[State::TimeWizFrozen]},
        [this]() { return calcSpeedEffect(); }));

    mParamSubs.push_back(params[TimeWizardParams::SpeedCost].subscribeTo(
        {params[TimeWizardParams::SpeedEffect]}, {},
        [this]() { return calcCost(); }));

    mParamSubs.push_back(params[TimeWizardParams::ClockSpeed].subscribeTo(
        {params[TimeWizardParams::FreezeBaseEffect],
         params[TimeWizardParams::FreezeEffect]},
        {}, [this]() { return calcClockSpeed(); }));

    mParamSubs.push_back(params[TimeWizardParams::SpeedEffect].subscribe(
        [this](const Number& val) {
            mImgAnimData.frame_ms =
                (unsigned int)(IMG.frame_ms / val.toFloat());
            if (mAnimTimerSub) {
                mAnimTimerSub->get<TimerObservable::DATA>() = mImgAnimData;
            }
        }));

    mParamSubs.push_back(states[State::BoughtPowerWizard].subscribe(
        [this](bool bought) { mBoostWizSpdUp->setActive(bought); }));

    mParamSubs.push_back(
        states[State::BoughtTimeWizard].subscribe([this](bool bought) {
            WizardSystem::GetHideObservable()->next(mId, !bought);
        }));

    mParamSubs.push_back(states[State::TimeWizFrozen].subscribe(
        [this](bool frozen) { onFreezeChange(frozen); }));
}

bool TimeWizard::onCostTimer(Timer& timer) {
    ParameterSystem::States states;

    if (!states[State::TimeWizFrozen].get() &&
        states[State::TimeWizActive].get()) {
        auto speedCost =
            ParameterSystem::Param<TIME_WIZARD>(TimeWizardParams::SpeedCost);
        auto money = ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic);
        money.set(money.get() * (1 - speedCost.get() * timer.length / 1000));
    }
    return true;
}

void TimeWizard::onRender(SDL_Renderer* r) {
    TextureBuilder tex;

    WizardBase::onRender(r);

    mFreezePb.dest = Rect(mPos->rect.x(), mPos->rect.y2(), mPos->rect.w(),
                          mPos->rect.h() / 15);
    tex.draw(mFreezePb);
}

void TimeWizard::onHide(bool hide) {
    WizardBase::onHide(hide);
    if (hide) {
        mActiveToggle->setLevel(0);
    }
    ParameterSystem::Param(State::TimeWizFrozen).set(false);
}

void TimeWizard::onT1Reset() {
    if (mCostTimerSub) {
        mCostTimerSub->get<TimerObservable::DATA>().reset();
    }
}

void TimeWizard::onFreezeChange(bool frozen) {
    mFreezeDelaySub.reset();
    mFreezeTimerSub.reset();
    if (frozen) {
        mFreezePb.mColor = CYAN;
        mTClock = ComponentFactory<TimeWizClock>::New(mPos->rect);
        mTimeLock = TimeSystem::GetUpdateObservable()->requestLock();
        if (!mHidden) {
            mFreezeTimerSub =
                ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
                    [this](Timer& t) {
                        ParameterSystem::Param(State::TimeWizFrozen).set(false);
                        return false;
                    },
                    [this](Time dt, Timer& timer) {
                        mFreezePb.set(1 - timer.getPercent());
                    },
                    Timer(ParameterSystem::Param<TIME_WIZARD>(
                              TimeWizardParams::FreezeDuration)
                              .get()
                              .toFloat()));
        }
    } else {
        mFreezePb.mColor = BLUE;
        mTClock.reset();
        TimeSystem::GetUpdateObservable()->releaseLock(mTimeLock);
        if (!mHidden) {
            mFreezeDelaySub =
                ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
                    [this](Timer& t) {
                        ParameterSystem::Param(State::TimeWizFrozen).set(true);
                        return false;
                    },
                    [this](Time dt, Timer& timer) {
                        mFreezePb.set(timer.getPercent());
                    },
                    Timer(ParameterSystem::Param<TIME_WIZARD>(
                              TimeWizardParams::FreezeDelay)
                              .get()
                              .toFloat()));
        }
    }
    updateImg();
}

void TimeWizard::onPowFireballHit(const PowerWizFireball& fireball) {
    ParameterSystem::Params<TIME_WIZARD> params;
    params[TimeWizardParams::TimeWarpEffect].set(fireball.getPower());
    WizardSystem::GetWizardEventObservable()->next(
        WizardSystem::Event::TimeWarp);
}

Number TimeWizard::calcFreezeEffect() {
    ParameterSystem::Params<TIME_WIZARD> params;
    return params[TimeWizardParams::FreezeBaseEffect].get() *
           params[TimeWizardParams::FreezeUp].get();
}

Number TimeWizard::calcSpeedEffect() {
    ParameterSystem::Params<TIME_WIZARD> params;
    ParameterSystem::States states;
    Number effect = 1;
    if (states[State::TimeWizActive].get() ||
        states[State::TimeWizFrozen].get()) {
        effect = params[TimeWizardParams::SpeedBaseEffect].get() +
                 params[TimeWizardParams::SpeedUp].get();
    }
    return effect;
}

Number TimeWizard::calcCost() {
    ParameterSystem::Params<TIME_WIZARD> params;
    Number effect = params[TimeWizardParams::SpeedEffect].get();
    Number result = min(effect - 1, .5) / 50;
    effect -= 1.5;
    if (effect > 0) {
        result += effect / 100;
    }
    return result;
}

Number TimeWizard::calcClockSpeed() {
    ParameterSystem::Params<TIME_WIZARD> params;
    return (params[TimeWizardParams::FreezeEffect].get() /
            params[TimeWizardParams::FreezeBaseEffect].get()) ^
           2;
}

void TimeWizard::updateImg() {
    Rect imgR = mImg.getRect();
    imgR.setPos(mPos->rect.cX(), mPos->rect.cY(), Rect::Align::CENTER);
    bool frozen = ParameterSystem::Param(State::TimeWizFrozen).get();
    mImg.set(frozen ? FREEZE_IMG : mImgAnimData);
    if (mAnimTimerSub) {
        mAnimTimerSub->get<TimerObservable::DATA>() =
            frozen ? FREEZE_IMG : mImgAnimData;
    }
    mPos->rect = mImg.setDest(imgR).getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);
}

void TimeWizard::setPos(float x, float y) {
    WizardBase::setPos(x, y);
    if (mTClock) {
        mTClock->setRect(mPos->rect);
    }
}
