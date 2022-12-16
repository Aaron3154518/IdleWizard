#include "TimeWizard.h"

// TimeWizard
TimeWizard::TimeWizard()
    : WizardBase(TIME_WIZARD), mImgAnimData(TimeWizardDefs::IMG()) {}

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
                mImg->nextFrame();
                WizardSystem::GetWizardImageObservable()->next(mId, mImg);
                if (ParameterSystem::Param(State::TimeWizFrozen).get()) {
                    t.length = mImg->getFrame() != 0
                                   ? TimeWizardDefs::FREEZE_IMG().frame_ms
                                   : (int)(rDist(gen) * 500) + 1000;
                }
                return true;
            },
            TimeWizardDefs::IMG());
    mT1ResetSub = WizardSystem::GetWizardEventObservable()->subscribe(
        [this]() { onT1Reset(); }, WizardSystem::Event::ResetT1);
    mPowFireballHitSub = PowerFireball::GetHitObservable()->subscribe(
        [this](const PowerFireball& fireball) { onPowFireballHit(fireball); },
        mId);
    mGlobHitSub =
        Glob::GetHitObservable()->subscribe([this]() { onGlobHit(); }, mPos);
    attachSubToVisibility(mCostTimerSub);
    attachSubToVisibility(mPowFireballHitSub);
    attachSubToVisibility(mGlobHitSub);
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
            auto active = ParameterSystem::Param(State::TimeWizActive);
            active.set(state == 1);
            tUp.setImage(active.get() ? TimeWizardDefs::ACTIVE_UP_IMG
                                      : WIZ_IMGS.at(mId));
        },
        2);
    mActiveToggle->setDescription(
        {"Consume {i} for a fire rate multiplier to {i}, {i}",
         {Money::GetMoneyIcon(UpgradeDefaults::CRYSTAL_MAGIC),
          IconSystem::Get(WizardDefs::IMG()),
          IconSystem::Get(PowerWizardDefs::IMG())}});
    mActiveToggle->setEffects(
        {params[TimeWizardParams::SpeedCost],
         params[TimeWizardParams::SpeedEffect]},
        {}, []() -> TextUpdateData {
            ParameterSystem::Params<TIME_WIZARD> params;
            std::stringstream ss;
            ss << params[TimeWizardParams::SpeedEffect].get() << "x\n-"
               << (params[TimeWizardParams::SpeedCost].get() * 100) << "%{i}/s";
            return {ss.str(),
                    {Money::GetMoneyIcon(UpgradeDefaults::CRYSTAL_MAGIC)}};
        });
    mActiveUp = mUpgrades->subscribe(mActiveToggle);

    // Speed upgrade
    UpgradePtr up =
        std::make_shared<Upgrade>(params[TimeWizardParams::SpeedUpLvl], 10);
    up->setImage(TimeWizardDefs::SPEED_UP_IMG);
    up->setDescription(
        {"Increase speed boost multiplier by +.05\nThis will also increase "
         "the {i} cost!",
         {Money::GetMoneyIcon(UpgradeDefaults::CRYSTAL_MAGIC)}});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                params[TimeWizardParams::SpeedUpCost]);
    up->setEffects(params[TimeWizardParams::SpeedUp],
                   UpgradeDefaults::AdditiveEffect);
    mParamSubs.push_back(UpgradeDefaults::subscribeT1UpCost(
        up->level(), params[TimeWizardParams::SpeedUpCost],
        [](const Number& lvl) { return 250 ^ (lvl / 12 + 1); }));
    mParamSubs.push_back(params[TimeWizardParams::SpeedUp].subscribeTo(
        up->level(), [](const Number& lvl) { return lvl * .05; }));
    mSpeedUp = mUpgrades->subscribe(up);

    // Speed effect upgrade
    up = std::make_shared<Upgrade>(params[TimeWizardParams::SpeedUpUpLvl], 8);
    up->setImage(TimeWizardDefs::POW_SPEED_UP_IMG);
    up->setDescription(
        {"Increases the speed boost upgrade effect by *1.1 and reduces the "
         "speed boost cost percent by *0.9"});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                params[TimeWizardParams::SpeedUpUpCost]);
    up->setEffects({params[TimeWizardParams::SpeedUpUp],
                    params[TimeWizardParams::SpeedUpCostUp]},
                   {}, []() -> TextUpdateData {
                       ParameterSystem::Params<TIME_WIZARD> params;
                       std::stringstream ss;
                       ss << "Speed "
                          << UpgradeDefaults::MultiplicativeEffect(
                                 params[TimeWizardParams::SpeedUpUp].get())
                                 .text
                          << "\nCost "
                          << UpgradeDefaults::MultiplicativeEffect(
                                 params[TimeWizardParams::SpeedUpCostUp].get())
                                 .text;
                       return {ss.str()};
                   });
    mParamSubs.push_back(params[TimeWizardParams::SpeedUpUpCost].subscribeTo(
        up->level(),
        [](const Number& lvl) { return Number(1, 7) * (2.1 ^ lvl); }));
    mParamSubs.push_back(params[TimeWizardParams::SpeedUpUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1.1 ^ lvl; }));
    mParamSubs.push_back(params[TimeWizardParams::SpeedUpCostUp].subscribeTo(
        up->level(), [](const Number& lvl) { return .9 ^ lvl; }));
    mSpeedUpUp = mUpgrades->subscribe(up);

    // Fireball Speed upgrade
    up = std::make_shared<Upgrade>(params[TimeWizardParams::FBSpeedUpLvl], 6);
    up->setImage(TimeWizardDefs::FB_SPEED_UP_IMG);
    up->setDescription(
        {"Increase {i} speed *1.075\nHigher speed gives more power",
         {IconSystem::Get(WizardDefs::FB_IMG())}});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                params[TimeWizardParams::FBSpeedCost]);
    up->setEffects(params[TimeWizardParams::FBSpeedUp],
                   UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(UpgradeDefaults::subscribeT1UpCost(
        up->level(), params[TimeWizardParams::FBSpeedCost],
        [](const Number& lvl) { return 100 * (2.5 ^ lvl); }));
    mParamSubs.push_back(params[TimeWizardParams::FBSpeedUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1.075 ^ lvl; }));
    mFBSpeedUp = mUpgrades->subscribe(up);

    // Freeze upgrade
    up = std::make_shared<Upgrade>(params[TimeWizardParams::FreezeUpLvl], 8);
    up->setImage(TimeWizardDefs::FREEZE_UP_IMG);
    up->setDescription({"Multiply unfreeze boost exponent by 1.03"});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                params[TimeWizardParams::FreezeUpCost]);
    up->setEffects(params[TimeWizardParams::FreezeUp],
                   UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(UpgradeDefaults::subscribeT1UpCost(
        up->level(), params[TimeWizardParams::FreezeUpCost],
        [](const Number& lvl) { return 150 * (1.6 ^ lvl); }));
    mParamSubs.push_back(params[TimeWizardParams::FreezeUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1.03 ^ lvl; }));
    mFreezeUp = mUpgrades->subscribe(up);
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
         params[TimeWizardParams::SpeedUp],
         params[TimeWizardParams::SpeedUpUp]},
        {states[State::TimeWizActive], states[State::TimeWizFrozen]},
        [this]() { return calcSpeedEffect(); }));

    mParamSubs.push_back(params[TimeWizardParams::SpeedCost].subscribeTo(
        {params[TimeWizardParams::SpeedEffect],
         params[TimeWizardParams::SpeedUpCostUp]},
        {}, [this]() { return calcCost(); }));

    mParamSubs.push_back(params[TimeWizardParams::ClockSpeed].subscribeTo(
        {params[TimeWizardParams::FreezeBaseEffect],
         params[TimeWizardParams::FreezeEffect]},
        {}, [this]() { return calcClockSpeed(); }));

    mParamSubs.push_back(params[TimeWizardParams::SpeedEffect].subscribe(
        [this](const Number& val) {
            mImgAnimData.frame_ms =
                (unsigned int)(TimeWizardDefs::IMG().frame_ms / val.toFloat());
            if (mAnimTimerSub) {
                mAnimTimerSub->get<TimerObservable::DATA>() = mImgAnimData;
            }
        }));

    mParamSubs.push_back(
        states[State::BoughtTimeWizard].subscribe([this](bool bought) {
            WizardSystem::GetHideObservable()->next(mId, !bought);
        }));

    mParamSubs.push_back(states[State::TimeWizFrozen].subscribe(
        [this](bool frozen) { onFreezeChange(frozen); }));

    // Upgrade unlock constraints
    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[TimeWizardParams::SpeedUpLvl]}, {states[State::BoughtSecondT1]},
        [this]() {
            mSpeedUpUp->setActive(
                ParameterSystem::Param<TIME_WIZARD>(
                    TimeWizardParams::SpeedUpLvl)
                        .get() > 0 &&
                ParameterSystem::Param(State::BoughtSecondT1).get());
        }));
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

void TimeWizard::onPowFireballHit(const PowerFireball& fireball) {
    ParameterSystem::Params<TIME_WIZARD> params;
    params[TimeWizardParams::TimeWarpEffect].set(fireball.getPower());
    WizardSystem::GetWizardEventObservable()->next(
        WizardSystem::Event::TimeWarp);
}

void TimeWizard::onGlobHit() {
    if (ParameterSystem::Param(State::TimeWizFrozen).get()) {
        Timer& t = mFreezeTimerSub->get<TimerObservable::DATA>();
        t.timer += t.length / 15;
    } else {
        Timer& t = mFreezeDelaySub->get<TimerObservable::DATA>();
        t.timer -= t.length / 50;
    }
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
                 (params[TimeWizardParams::SpeedUp].get() *
                  params[TimeWizardParams::SpeedUpUp].get());
    }
    return effect;
}

Number TimeWizard::calcCost() {
    ParameterSystem::Params<TIME_WIZARD> params;
    Number effect = params[TimeWizardParams::SpeedEffect].get();
    Number result = min(effect - 1, .5) / 50;
    effect -= 1.5;
    if (effect > 0) {
        result += effect / 50;
    }
    result *= params[TimeWizardParams::SpeedUpCostUp].get();
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
    mImg.set(frozen ? TimeWizardDefs::FREEZE_IMG() : mImgAnimData);
    if (mAnimTimerSub) {
        mAnimTimerSub->get<TimerObservable::DATA>() =
            frozen ? TimeWizardDefs::FREEZE_IMG() : mImgAnimData;
    }
    mImg.setDest(imgR);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);
}

void TimeWizard::setPos(float x, float y) {
    WizardBase::setPos(x, y);
    if (mTClock) {
        mTClock->setRect(mPos->rect);
    }
}
