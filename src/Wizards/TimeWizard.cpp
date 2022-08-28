#include "TimeWizard.h"

// TimeWizard
const unsigned int TimeWizard::MSPF = 100, TimeWizard::NUM_FRAMES = 8;
const unsigned int TimeWizard::FROZEN_MSPF = 100,
                   TimeWizard::FROZEN_NUM_FRAMES = 9;

const std::string TimeWizard::IMG = "res/wizards/time_wizard_ss.png";
const std::string TimeWizard::FREEZE_IMG =
    "res/wizards/time_wizard_frozen_ss.png";
const std::string TimeWizard::FREEZE_UP_IMG =
    "res/upgrades/time_freeze_upgrade.png";
const std::string TimeWizard::SPEED_UP_IMG = "res/upgrades/speed_upgrade.png";

void TimeWizard::setDefaults() {
    using WizardSystem::ResetTier;

    ParameterSystem::Params<TIME_WIZARD> params;

    params[TimeWizardParams::SpeedBaseEffect]->init(1.5);
    params[TimeWizardParams::FreezeBaseEffect]->init(1.1);
    params[TimeWizardParams::FreezeDelay]->init(3000);
    params[TimeWizardParams::FreezeDuration]->init(10000);

    params[TimeWizardParams::SpeedUpLvl]->init(ResetTier::T1);
    params[TimeWizardParams::FreezeUpLvl]->init(ResetTier::T1);

    ParameterSystem::States states;

    states[State::TimeWizActive]->init(false, ResetTier::T1);
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
                if (TimeSystem::Frozen(TimeSystem::FreezeType::TIME_WIZARD)) {
                    t.length = mImg.getFrame() != 0
                                   ? FROZEN_MSPF
                                   : (int)(rDist(gen) * 500) + 1000;
                } else {
                    t.length = MSPF;
                }
                return true;
            },
            Timer(MSPF));
    attachSubToVisibility(mCostTimerSub);
}
void TimeWizard::setUpgrades() {
    ParameterSystem::Params<TIME_WIZARD> params;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(mId);
    dUp->setDescription("Speed multiplier");
    dUp->setEffects({params[TimeWizardParams::FreezeDelay],
                     params[TimeWizardParams::FreezeDuration],
                     params[TimeWizardParams::FreezeEffect]},
                    {}, []() {
                        ParameterSystem::Params<TIME_WIZARD> params;
                        std::stringstream ss;
                        ss << "Cooldown: "
                           << params[TimeWizardParams::FreezeDelay].get() / 1000
                           << "s\nDuration: "
                           << params[TimeWizardParams::FreezeDuration].get() /
                                  1000
                           << "s\nUnfreeze Effect: Power ^ "
                           << params[TimeWizardParams::FreezeEffect].get();
                        return ss.str();
                    });
    mEffectDisplay = mUpgrades->subscribe(dUp);

    // Active toggle
    mActiveToggle = std::make_shared<Toggle>(
        [this](unsigned int state, Toggle& tUp) {
            mActive = state == 1;
            auto sub = mSpeedEffectSub.lock();
            if (sub) {
                sub->get<0>()();
            }
            updateImg();
        },
        2);
    mActiveToggle->setImage(WIZ_IMGS.at(mId));
    mActiveToggle->setDescription(
        "Consume magic for a fire rate multiplier to all Wizards");
    mActiveToggle->setEffects(
        {params[TimeWizardParams::SpeedCost],
         params[TimeWizardParams::SpeedEffect]},
        {}, []() {
            ParameterSystem::Params<TIME_WIZARD> params;
            std::stringstream ss;
            ss << params[TimeWizardParams::SpeedEffect].get() << "x\n-$"
               << params[TimeWizardParams::SpeedCost].get() << "/s";
            return ss.str();
        });
    mActiveUp = mUpgrades->subscribe(mActiveToggle);

    // Freeze upgrade
    UpgradePtr up =
        std::make_shared<Upgrade>(params[TimeWizardParams::FreezeUpLvl], 8);
    up->setImage(FREEZE_UP_IMG);
    up->setDescription("Multiply unfreeze boost exponent by 1.2");
    up->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                params[TimeWizardParams::FreezeUpCost],
                [](const Number& lvl) { return 150 * (1.6 ^ lvl); });
    up->setEffect(
        params[TimeWizardParams::FreezeUp],
        [](const Number& lvl) { return 1.02 ^ lvl; },
        Upgrade::Defaults::MultiplicativeEffect);
    mFreezeUp = mUpgrades->subscribe(up);

    // Speed upgrade
    up = std::make_shared<Upgrade>(params[TimeWizardParams::SpeedUpLvl], 10);
    up->setImage(SPEED_UP_IMG);
    up->setDescription(
        "Increase speed boost multiplier by +.05\nThis will also increase "
        "the magic cost!");
    up->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                params[TimeWizardParams::SpeedUpCost],
                [](const Number& lvl) { return 250 ^ (lvl / 12 + 1); });
    up->setEffect(
        params[TimeWizardParams::SpeedUp],
        [](const Number& lvl) { return lvl * .05; },
        Upgrade::Defaults::AdditiveEffect);
    mSpeedUp = mUpgrades->subscribe(up);
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
        {states[State::TimeWizActive]},
        [this]() { return calcSpeedEffect(); }));

    mParamSubs.push_back(params[TimeWizardParams::SpeedCost].subscribeTo(
        {params[TimeWizardParams::SpeedEffect]}, {},
        [this]() { return calcCost(); }));
    mSpeedEffectSub = mParamSubs.back();

    mParamSubs.push_back(params[TimeWizardParams::ClockSpeed].subscribeTo(
        {params[TimeWizardParams::FreezeBaseEffect],
         params[TimeWizardParams::FreezeEffect]},
        {}, [this]() { return calcClockSpeed(); }));

    mParamSubs.push_back(params[TimeWizardParams::SpeedEffect].subscribe(
        [this](const Number& val) {
            if (mAnimTimerSub) {
                mAnimTimerSub->get<TimerObservable::DATA>().length =
                    (int)(MSPF / val.toFloat());
            }
        }));

    mParamSubs.push_back(
        states[State::BoughtTimeWizard].subscribe([this](bool bought) {
            WizardSystem::GetHideObservable()->next(mId, !bought);
        }));
}

bool TimeWizard::onCostTimer(Timer& timer) {
    bool prevAfford = mCanAfford;
    mCanAfford = false;
    if (mActive) {
        auto speedParam =
            ParameterSystem::Param<TIME_WIZARD>(TimeWizardParams::SpeedCost);
        auto magicParam = ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic);
        Number cost = speedParam.get() * timer.length / 1000;
        Number money = magicParam.get();
        if (cost <= money) {
            magicParam.set(money - cost);
            mCanAfford = true;
        }
    }
    if (mCanAfford != prevAfford) {
        ParameterSystem::Param(State::TimeWizActive).set(mCanAfford);
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

void TimeWizard::onHide(WizardId id, bool hide) {
    WizardBase::onHide(id, hide);
    if (id == mId) {
        if (hide) {
            TimeSystem::Unfreeze(TimeSystem::FreezeType::TIME_WIZARD);
            mFreezeDelaySub.reset();
            mFreezeTimerSub.reset();
            mActiveToggle->setLevel(0);
        } else {
            startFreezeCycle();
        }
    }
}

void TimeWizard::onReset(WizardSystem::ResetTier tier) {
    if (mCostTimerSub) {
        mCostTimerSub->get<TimerObservable::DATA>().reset();
    }
}

bool TimeWizard::startFreeze(Timer& timer) {
    TimeSystem::Freeze(TimeSystem::FreezeType::TIME_WIZARD);
    mFreezeTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            [this](Timer& t) { return endFreeze(t); },
            [this](Time dt, Timer& timer) { onFreezeTimer(dt, timer); },
            Timer(ParameterSystem::Param<TIME_WIZARD>(
                      TimeWizardParams::FreezeDuration)
                      .get()
                      .toFloat()));
    mFreezePb.mColor = CYAN;
    updateImg();
    mTClock = ComponentFactory<TimeWizClock>::New(mPos->rect);
    return false;
}

void TimeWizard::onFreezeTimer(Time dt, Timer& timer) {
    mFreezePb.set(1 - timer.getPercent());
}

bool TimeWizard::endFreeze(Timer& timer) {
    TimeSystem::Unfreeze(TimeSystem::FreezeType::TIME_WIZARD);
    mTClock.reset();
    startFreezeCycle();
    return false;
}

void TimeWizard::startFreezeCycle() {
    mFreezeDelaySub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            [this](Timer& t) { return startFreeze(t); },
            [this](Time dt, Timer& timer) {
                mFreezePb.set(timer.getPercent());
            },
            Timer(ParameterSystem::Param<TIME_WIZARD>(
                      TimeWizardParams::FreezeDelay)
                      .get()
                      .toFloat()));
    mFreezePb.mColor = BLUE;
    updateImg();
}

Number TimeWizard::calcFreezeEffect() {
    ParameterSystem::Params<TIME_WIZARD> params;
    return params[TimeWizardParams::FreezeBaseEffect].get() *
           params[TimeWizardParams::FreezeUp].get();
}

Number TimeWizard::calcSpeedEffect() {
    ParameterSystem::Params<TIME_WIZARD> params;
    Number effect = 1;
    if (ParameterSystem::Param(State::TimeWizActive).get()) {
        effect = params[TimeWizardParams::SpeedBaseEffect].get() +
                 params[TimeWizardParams::SpeedUp].get();
    }
    return effect;
}

Number TimeWizard::calcCost() {
    ParameterSystem::Params<TIME_WIZARD> params;
    Number effect = params[TimeWizardParams::SpeedEffect].get();
    return 2 * (effect - 1) * (10 ^ (effect ^ (effect / 3)));
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
    bool frozen = TimeSystem::Frozen(TimeSystem::FreezeType::TIME_WIZARD);
    if (frozen) {
        mImg.set(FREEZE_IMG, FROZEN_NUM_FRAMES);
    } else {
        mImg.set(IMG, NUM_FRAMES);
    }
    if (mAnimTimerSub) {
        Timer& timer = mAnimTimerSub->get<TimerObservable::DATA>();
        timer.length = frozen ? FROZEN_MSPF : MSPF;
        timer.timer = 0;
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
