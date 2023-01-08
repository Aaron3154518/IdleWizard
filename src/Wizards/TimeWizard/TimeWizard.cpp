#include "TimeWizard.h"

namespace TimeWizard {
// TimeWizard
TimeWizard::TimeWizard()
    : WizardBase(TIME_WIZARD), mImgAnimData(Constants::IMG()) {}

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
                if (Params::get(Param::Frozen).get()) {
                    t.length = mImg->getFrame() != 0
                                   ? Constants::FREEZE_IMG().frame_ms
                                   : (int)(rDist(gen) * 500) + 1000;
                }
                return true;
            },
            Constants::IMG());
    mT1ResetSub = WizardSystem::GetWizardEventObservable()->subscribe(
        [this]() { onT1Reset(); }, WizardSystem::Event::ResetT1);
    mPowFireballHitSub =
        PowerWizard::FireballList::GetHitObservable()->subscribe(
            [this](const PowerWizard::Fireball& fb) { onPowFireballHit(fb); },
            [this](const PowerWizard::Fireball& fb) {
                return PowerWizard::Fireball::filter(fb, mId);
            },
            mPos);
    mGlobHitSub = PoisonWizard::Glob::GetHitObservable()->subscribe(
        [this]() { onGlobHit(); }, mPos);
    attachSubToVisibility(mCostTimerSub);
    attachSubToVisibility(mPowFireballHitSub);
    attachSubToVisibility(mGlobHitSub);
}
void TimeWizard::setUpgrades() {
    Params params;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(mId);
    dUp->setDescription({"Speed multiplier"});
    dUp->setEffects(
        {params[Param::FreezeDelay], params[Param::FreezeDuration],
         params[Param::FreezeEffect]},
        {}, []() -> TextUpdateData {
            Params params;
            std::stringstream ss;
            ss << "Cooldown: " << params[Param::FreezeDelay].get() / 1000
               << "s\nDuration: " << params[Param::FreezeDuration].get() / 1000
               << "s\nUnfreeze Effect: Power ^ "
               << params[Param::FreezeEffect].get();
            return {ss.str()};
        });
    mEffectDisplay = mUpgrades->subscribe(dUp);

    // Active toggle
    mActiveToggle = std::make_shared<Toggle>(
        [this](unsigned int state, Toggle& tUp) {
            auto active = Params::get(Param::SpeedToggleActive);
            active.set(state == 1);
        },
        2);
    mActiveToggle->setDescription(
        {"Consume {i} for a fire rate multiplier to {i}, {i}",
         {MoneyIcons::Get(UpgradeDefaults::CRYSTAL_MAGIC),
          IconSystem::Get(Wizard::Constants::IMG()),
          IconSystem::Get(PowerWizard::Constants::IMG())}});
    mActiveToggle->setEffects(
        {params[Param::SpeedCost], params[Param::SpeedEffect]}, {},
        []() -> TextUpdateData {
            Params params;
            std::stringstream ss;
            ss << params[Param::SpeedEffect].get() << "x\n-"
               << (params[Param::SpeedCost].get() * 100) << "%{i}/s";
            return {ss.str(),
                    {MoneyIcons::Get(UpgradeDefaults::CRYSTAL_MAGIC)}};
        });
    mActiveUp = mUpgrades->subscribe(mActiveToggle);

    // Speed upgrade
    UpgradePtr up = std::make_shared<Upgrade>(params[Param::SpeedUpLvl], 10);
    up->setImage(Constants::SPEED_UP_IMG);
    up->setDescription(
        {"Increase speed boost multiplier by +.05\nThis will also increase "
         "the {i} cost!",
         {MoneyIcons::Get(UpgradeDefaults::CRYSTAL_MAGIC)}});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC, params[Param::SpeedUpCost]);
    up->setEffects(params[Param::SpeedUp], UpgradeDefaults::AdditiveEffect);
    mParamSubs.push_back(UpgradeDefaults::subscribeT1UpCost(
        up->level(), params[Param::SpeedUpCost],
        [](const Number& lvl) { return 250 ^ (lvl / 12 + 1); }));
    mParamSubs.push_back(params[Param::SpeedUp].subscribeTo(
        up->level(), [](const Number& lvl) { return lvl * .05; }));
    mSpeedUp = mUpgrades->subscribe(up);

    // Speed effect upgrade
    up = std::make_shared<Upgrade>(params[Param::SpeedUpUpLvl], 8);
    up->setImage(Constants::POW_SPEED_UP_IMG);
    up->setDescription(
        {"Increases the speed boost upgrade effect by *1.1 and reduces the "
         "speed boost cost percent by *0.9"});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC, params[Param::SpeedUpUpCost]);
    up->setEffects({params[Param::SpeedUpUp], params[Param::SpeedUpCostUp]}, {},
                   []() -> TextUpdateData {
                       Params params;
                       std::stringstream ss;
                       ss << "Speed "
                          << UpgradeDefaults::MultiplicativeEffect(
                                 params[Param::SpeedUpUp].get())
                                 .text
                          << "\nCost "
                          << UpgradeDefaults::MultiplicativeEffect(
                                 params[Param::SpeedUpCostUp].get())
                                 .text;
                       return {ss.str()};
                   });
    mParamSubs.push_back(params[Param::SpeedUpUpCost].subscribeTo(
        up->level(),
        [](const Number& lvl) { return Number(1, 7) * (2.1 ^ lvl); }));
    mParamSubs.push_back(params[Param::SpeedUpUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1.1 ^ lvl; }));
    mParamSubs.push_back(params[Param::SpeedUpCostUp].subscribeTo(
        up->level(), [](const Number& lvl) { return .9 ^ lvl; }));
    mSpeedUpUp = mUpgrades->subscribe(up);

    // Fireball Speed upgrade
    up = std::make_shared<Upgrade>(params[Param::FBSpeedUpLvl], 6);
    up->setImage(Constants::FB_SPEED_UP_IMG);
    up->setDescription(
        {"Increase {i} speed *1.075\nHigher speed gives more power",
         {IconSystem::Get(Wizard::Constants::FB_IMG())}});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC, params[Param::FBSpeedCost]);
    up->setEffects(params[Param::FBSpeedUp],
                   UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(UpgradeDefaults::subscribeT1UpCost(
        up->level(), params[Param::FBSpeedCost],
        [](const Number& lvl) { return 100 * (2.5 ^ lvl); }));
    mParamSubs.push_back(params[Param::FBSpeedUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1.075 ^ lvl; }));
    mFBSpeedUp = mUpgrades->subscribe(up);

    // Freeze upgrade
    up = std::make_shared<Upgrade>(params[Param::FreezeUpLvl], 8);
    up->setImage(Constants::FREEZE_UP_IMG);
    up->setDescription({"Multiply unfreeze boost exponent by 1.03"});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC, params[Param::FreezeUpCost]);
    up->setEffects(params[Param::FreezeUp],
                   UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(UpgradeDefaults::subscribeT1UpCost(
        up->level(), params[Param::FreezeUpCost],
        [](const Number& lvl) { return 150 * (1.6 ^ lvl); }));
    mParamSubs.push_back(params[Param::FreezeUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1.03 ^ lvl; }));
    mFreezeUp = mUpgrades->subscribe(up);
}
void TimeWizard::setParamTriggers() {
    Params params;
    Crystal::Params cryParams;
    RobotWizard::Params roboParams;

    mParamSubs.push_back(params[Param::FreezeEffect].subscribeTo(
        {params[Param::FreezeBaseEffect], params[Param::FreezeUp]}, {},
        [this]() { return calcFreezeEffect(); }));

    mParamSubs.push_back(params[Param::SpeedEffect].subscribeTo(
        {params[Param::SpeedBaseEffect], params[Param::SpeedUp],
         params[Param::SpeedUpUp]},
        {params[Param::SpeedActive], params[Param::Frozen]},
        [this]() { return calcSpeedEffect(); }));

    mParamSubs.push_back(params[Param::SpeedCost].subscribeTo(
        {params[Param::SpeedEffect], params[Param::SpeedUpCostUp]},
        {roboParams[RobotWizard::Param::BoughtNoTimeCostUp]},
        [this]() { return calcCost(); }));

    mParamSubs.push_back(params[Param::ClockSpeed].subscribeTo(
        {params[Param::FreezeBaseEffect], params[Param::FreezeEffect]}, {},
        [this]() { return calcClockSpeed(); }));

    mParamSubs.push_back(params[Param::SpeedActive].subscribeTo(
        {},
        {params[Param::SpeedToggleActive],
         roboParams[RobotWizard::Param::BoughtNoTimeCostUp]},
        [params, roboParams]() {
            return params[Param::SpeedToggleActive].get() ||
                   roboParams[RobotWizard::Param::BoughtNoTimeCostUp].get();
        }));

    mParamSubs.push_back(
        params[Param::SpeedActive].subscribe([this](bool active) {
            mActiveToggle->setImage(active ? Constants::ACTIVE_UP_IMG
                                           : WIZ_IMGS.at(mId));
        }));

    mParamSubs.push_back(
        params[Param::SpeedEffect].subscribe([this](const Number& val) {
            mImgAnimData.frame_ms =
                (unsigned int)(Constants::IMG().frame_ms / val.toFloat());
            if (mAnimTimerSub) {
                mAnimTimerSub->get<TimerObservable::DATA>() = mImgAnimData;
            }
        }));

    mParamSubs.push_back(cryParams[Crystal::Param::BoughtTimeWizard].subscribe(
        [this](bool bought) {
            WizardSystem::GetHideObservable()->next(mId, !bought);
        }));

    mParamSubs.push_back(params[Param::Frozen].subscribe(
        [this](bool frozen) { onFreezeChange(frozen); }));

    // Upgrade unlock constraints
    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[Param::SpeedUpLvl]},
        {cryParams[Crystal::Param::BoughtSecondT1]}, [this]() {
            mSpeedUpUp->setActive(
                Params::get(Param::SpeedUpLvl).get() > 0 &&
                Crystal::Params::get(Crystal::Param::BoughtSecondT1).get());
        }));
}

bool TimeWizard::onCostTimer(Timer& timer) {
    Params params;

    if (!params[Param::Frozen].get() && params[Param::SpeedActive].get()) {
        auto speedCost = Params::get(Param::SpeedCost);
        auto money = Crystal::Params::get(Crystal::Param::Magic);
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

void TimeWizard::onClick(Event::MouseButton b, bool clicked) {
    static bool _prev_clicked = false;

    WizardBase::onClick(b, clicked);

    if (_prev_clicked && clicked) {
        Params::get(Param::Frozen).set(!Params::get(Param::Frozen).get());
    }
    _prev_clicked = clicked;
}

void TimeWizard::onHide(bool hide) {
    WizardBase::onHide(hide);
    if (hide) {
        mActiveToggle->setLevel(0);
    }
    Params::get(Param::Frozen).set(false);
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
        mTClock = ComponentFactory<Clock>::New(mPos->rect);
        mTimeLock = TimeSystem::GetUpdateObservable()->requestLock();
        if (!mHidden) {
            mFreezeTimerSub =
                ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
                    [this](Timer& t) {
                        Params::get(Param::Frozen).set(false);
                        return false;
                    },
                    [this](Time dt, Timer& timer) {
                        mFreezePb.set(1 - timer.getPercent());
                    },
                    Timer(Params::get(Param::FreezeDuration).get().toFloat()));
        }
    } else {
        mFreezePb.mColor = BLUE;
        mTClock.reset();
        TimeSystem::GetUpdateObservable()->releaseLock(mTimeLock);
        if (!mHidden) {
            mFreezeDelaySub =
                ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
                    [this](Timer& t) {
                        Params::get(Param::Frozen).set(true);
                        return false;
                    },
                    [this](Time dt, Timer& timer) {
                        mFreezePb.set(timer.getPercent());
                    },
                    Timer(Params::get(Param::FreezeDelay).get().toFloat()));
        }
    }
    updateImg();
}

void TimeWizard::onPowFireballHit(const PowerWizard::Fireball& fireball) {
    Params params;
    params[Param::TimeWarpEffect].set(fireball.getPower());
    WizardSystem::GetWizardEventObservable()->next(
        WizardSystem::Event::TimeWarp);
}

void TimeWizard::onGlobHit() {
    if (Params::get(Param::Frozen).get()) {
        Timer& t = mFreezeTimerSub->get<TimerObservable::DATA>();
        t.timer += t.length / 15;
    } else {
        Timer& t = mFreezeDelaySub->get<TimerObservable::DATA>();
        t.timer -= t.length / 50;
    }
}

Number TimeWizard::calcFreezeEffect() {
    Params params;
    return params[Param::FreezeBaseEffect].get() *
           params[Param::FreezeUp].get();
}

Number TimeWizard::calcSpeedEffect() {
    Params params;
    Number effect = 1;
    if (params[Param::SpeedActive].get() || params[Param::Frozen].get()) {
        effect =
            params[Param::SpeedBaseEffect].get() +
            (params[Param::SpeedUp].get() * params[Param::SpeedUpUp].get());
    }
    return effect;
}

Number TimeWizard::calcCost() {
    if (RobotWizard::Params::get(RobotWizard::Param::BoughtNoTimeCostUp)
            .get()) {
        return 0;
    }

    Params params;
    Number effect = params[Param::SpeedEffect].get();
    Number result = Number::min(effect - 1, .5) / 50;
    effect -= 1.5;
    if (effect > 0) {
        result += effect / 50;
    }
    result *= params[Param::SpeedUpCostUp].get();
    return result;
}

Number TimeWizard::calcClockSpeed() {
    Params params;
    return (params[Param::FreezeEffect].get() /
            params[Param::FreezeBaseEffect].get()) ^
           2;
}

void TimeWizard::updateImg() {
    Rect imgR = mImg.getRect();
    imgR.setPos(mPos->rect.cX(), mPos->rect.cY(), Rect::Align::CENTER);
    bool frozen = Params::get(Param::Frozen).get();
    mImg.set(frozen ? Constants::FREEZE_IMG() : mImgAnimData);
    if (mAnimTimerSub) {
        mAnimTimerSub->get<TimerObservable::DATA>() =
            frozen ? Constants::FREEZE_IMG() : mImgAnimData;
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
}  // namespace TimeWizard
