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
                if (ParameterSystem::Param(Param::TimeWizFrozen).get()) {
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
    TimeWizard::Params params;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(mId);
    dUp->setDescription({"Speed multiplier"});
    dUp->setEffects({params[TimeWizard::Param::FreezeDelay],
                     params[TimeWizard::Param::FreezeDuration],
                     params[TimeWizard::Param::FreezeEffect]},
                    {}, []() -> TextUpdateData {
                        TimeWizard::Params params;
                        std::stringstream ss;
                        ss << "Cooldown: "
                           << params[TimeWizard::Param::FreezeDelay].get() / 1000
                           << "s\nDuration: "
                           << params[TimeWizard::Param::FreezeDuration].get() /
                                  1000
                           << "s\nUnfreeze Effect: Power ^ "
                           << params[TimeWizard::Param::FreezeEffect].get();
                        return {ss.str()};
                    });
    mEffectDisplay = mUpgrades->subscribe(dUp);

    // Active toggle
    mActiveToggle = std::make_shared<Toggle>(
        [this](unsigned int state, Toggle& tUp) {
            auto active = ParameterSystem::Param(Param::TimeWizActive);
            active.set(state == 1);
            tUp.setImage(active.get() ? Constants::ACTIVE_UP_IMG
                                      : WIZ_IMGS.at(mId));
        },
        2);
    mActiveToggle->setDescription(
        {"Consume {i} for a fire rate multiplier to {i}, {i}",
         {MoneyIcons::GetMoneyIcon(UpgradeDefaults::CRYSTAL_MAGIC),
          IconSystem::Get(Wizard::Constants::IMG()),
          IconSystem::Get(PowerWizard::Constants::IMG())}});
    mActiveToggle->setEffects(
        {params[TimeWizard::Param::SpeedCost],
         params[TimeWizard::Param::SpeedEffect]},
        {}, []() -> TextUpdateData {
            TimeWizard::Params params;
            std::stringstream ss;
            ss << params[TimeWizard::Param::SpeedEffect].get() << "x\n-"
               << (params[TimeWizard::Param::SpeedCost].get() * 100) << "%{i}/s";
            return {ss.str(),
                    {MoneyIcons::GetMoneyIcon(UpgradeDefaults::CRYSTAL_MAGIC)}};
        });
    mActiveUp = mUpgrades->subscribe(mActiveToggle);

    // Speed upgrade
    UpgradePtr up =
        std::make_shared<Upgrade>(params[TimeWizard::Param::SpeedUpLvl], 10);
    up->setImage(Constants::SPEED_UP_IMG);
    up->setDescription(
        {"Increase speed boost multiplier by +.05\nThis will also increase "
         "the {i} cost!",
         {MoneyIcons::GetMoneyIcon(UpgradeDefaults::CRYSTAL_MAGIC)}});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                params[TimeWizard::Param::SpeedUpCost]);
    up->setEffects(params[TimeWizard::Param::SpeedUp],
                   UpgradeDefaults::AdditiveEffect);
    mParamSubs.push_back(UpgradeDefaults::subscribeT1UpCost(
        up->level(), params[TimeWizard::Param::SpeedUpCost],
        [](const Number& lvl) { return 250 ^ (lvl / 12 + 1); }));
    mParamSubs.push_back(params[TimeWizard::Param::SpeedUp].subscribeTo(
        up->level(), [](const Number& lvl) { return lvl * .05; }));
    mSpeedUp = mUpgrades->subscribe(up);

    // Speed effect upgrade
    up = std::make_shared<Upgrade>(params[TimeWizard::Param::SpeedUpUpLvl], 8);
    up->setImage(Constants::POW_SPEED_UP_IMG);
    up->setDescription(
        {"Increases the speed boost upgrade effect by *1.1 and reduces the "
         "speed boost cost percent by *0.9"});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                params[TimeWizard::Param::SpeedUpUpCost]);
    up->setEffects({params[TimeWizard::Param::SpeedUpUp],
                    params[TimeWizard::Param::SpeedUpCostUp]},
                   {}, []() -> TextUpdateData {
                       TimeWizard::Params params;
                       std::stringstream ss;
                       ss << "Speed "
                          << UpgradeDefaults::MultiplicativeEffect(
                                 params[TimeWizard::Param::SpeedUpUp].get())
                                 .text
                          << "\nCost "
                          << UpgradeDefaults::MultiplicativeEffect(
                                 params[TimeWizard::Param::SpeedUpCostUp].get())
                                 .text;
                       return {ss.str()};
                   });
    mParamSubs.push_back(params[TimeWizard::Param::SpeedUpUpCost].subscribeTo(
        up->level(),
        [](const Number& lvl) { return Number(1, 7) * (2.1 ^ lvl); }));
    mParamSubs.push_back(params[TimeWizard::Param::SpeedUpUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1.1 ^ lvl; }));
    mParamSubs.push_back(params[TimeWizard::Param::SpeedUpCostUp].subscribeTo(
        up->level(), [](const Number& lvl) { return .9 ^ lvl; }));
    mSpeedUpUp = mUpgrades->subscribe(up);

    // Fireball Speed upgrade
    up = std::make_shared<Upgrade>(params[TimeWizard::Param::FBSpeedUpLvl], 6);
    up->setImage(Constants::FB_SPEED_UP_IMG);
    up->setDescription(
        {"Increase {i} speed *1.075\nHigher speed gives more power",
         {IconSystem::Get(Wizard::Constants::FB_IMG())}});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                params[TimeWizard::Param::FBSpeedCost]);
    up->setEffects(params[TimeWizard::Param::FBSpeedUp],
                   UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(UpgradeDefaults::subscribeT1UpCost(
        up->level(), params[TimeWizard::Param::FBSpeedCost],
        [](const Number& lvl) { return 100 * (2.5 ^ lvl); }));
    mParamSubs.push_back(params[TimeWizard::Param::FBSpeedUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1.075 ^ lvl; }));
    mFBSpeedUp = mUpgrades->subscribe(up);

    // Freeze upgrade
    up = std::make_shared<Upgrade>(params[TimeWizard::Param::FreezeUpLvl], 8);
    up->setImage(Constants::FREEZE_UP_IMG);
    up->setDescription({"Multiply unfreeze boost exponent by 1.03"});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                params[TimeWizard::Param::FreezeUpCost]);
    up->setEffects(params[TimeWizard::Param::FreezeUp],
                   UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(UpgradeDefaults::subscribeT1UpCost(
        up->level(), params[TimeWizard::Param::FreezeUpCost],
        [](const Number& lvl) { return 150 * (1.6 ^ lvl); }));
    mParamSubs.push_back(params[TimeWizard::Param::FreezeUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1.03 ^ lvl; }));
    mFreezeUp = mUpgrades->subscribe(up);
}
void TimeWizard::setParamTriggers() {
    TimeWizard::Params params;

    mParamSubs.push_back(params[TimeWizard::Param::FreezeEffect].subscribeTo(
        {params[TimeWizard::Param::FreezeBaseEffect],
         params[TimeWizard::Param::FreezeUp]},
        {}, [this]() { return calcFreezeEffect(); }));

    mParamSubs.push_back(params[TimeWizard::Param::SpeedEffect].subscribeTo(
        {params[TimeWizard::Param::SpeedBaseEffect],
         params[TimeWizard::Param::SpeedUp],
         params[TimeWizard::Param::SpeedUpUp]},
        {params[Param::TimeWizActive], params[Param::TimeWizFrozen]},
        [this]() { return calcSpeedEffect(); }));

    mParamSubs.push_back(params[TimeWizard::Param::SpeedCost].subscribeTo(
        {params[TimeWizard::Param::SpeedEffect],
         params[TimeWizard::Param::SpeedUpCostUp]},
        {}, [this]() { return calcCost(); }));

    mParamSubs.push_back(params[TimeWizard::Param::ClockSpeed].subscribeTo(
        {params[TimeWizard::Param::FreezeBaseEffect],
         params[TimeWizard::Param::FreezeEffect]},
        {}, [this]() { return calcClockSpeed(); }));

    mParamSubs.push_back(params[TimeWizard::Param::SpeedEffect].subscribe(
        [this](const Number& val) {
            mImgAnimData.frame_ms =
                (unsigned int)(Constants::IMG().frame_ms / val.toFloat());
            if (mAnimTimerSub) {
                mAnimTimerSub->get<TimerObservable::DATA>() = mImgAnimData;
            }
        }));

    mParamSubs.push_back(
        params[Crystal::Param::BoughtTimeWizard].subscribe([this](bool bought) {
            WizardSystem::GetHideObservable()->next(mId, !bought);
        }));

    mParamSubs.push_back(params[Param::TimeWizFrozen].subscribe(
        [this](bool frozen) { onFreezeChange(frozen); }));

    // Upgrade unlock constraints
    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[TimeWizard::Param::SpeedUpLvl]}, {params[Param::BoughtSecondT1]},
        [this]() {
            mSpeedUpUp->setActive(
                TimeWizard::Params::get(
                    TimeWizard::Param::SpeedUpLvl)
                        .get() > 0 &&
                ParameterSystem::Param(Param::BoughtSecondT1).get());
        }));
}

bool TimeWizard::onCostTimer(Timer& timer) {

    if (!params[Param::TimeWizFrozen].get() &&
        params[Param::TimeWizActive].get()) {
        auto speedCost =
            TimeWizard::Params::get(TimeWizard::Param::SpeedCost);
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

void TimeWizard::onHide(bool hide) {
    WizardBase::onHide(hide);
    if (hide) {
        mActiveToggle->setLevel(0);
    }
    ParameterSystem::Param(Param::TimeWizFrozen).set(false);
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
                        ParameterSystem::Param(Param::TimeWizFrozen).set(false);
                        return false;
                    },
                    [this](Time dt, Timer& timer) {
                        mFreezePb.set(1 - timer.getPercent());
                    },
                    Timer(TimeWizard::Params::get(
                              TimeWizard::Param::FreezeDuration)
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
                        ParameterSystem::Param(Param::TimeWizFrozen).set(true);
                        return false;
                    },
                    [this](Time dt, Timer& timer) {
                        mFreezePb.set(timer.getPercent());
                    },
                    Timer(TimeWizard::Params::get(
                              TimeWizard::Param::FreezeDelay)
                              .get()
                              .toFloat()));
        }
    }
    updateImg();
}

void TimeWizard::onPowFireballHit(const PowerWizard::Fireball& fireball) {
    TimeWizard::Params params;
    params[TimeWizard::Param::TimeWarpEffect].set(fireball.getPower());
    WizardSystem::GetWizardEventObservable()->next(
        WizardSystem::Event::TimeWarp);
}

void TimeWizard::onGlobHit() {
    if (ParameterSystem::Param(Param::TimeWizFrozen).get()) {
        Timer& t = mFreezeTimerSub->get<TimerObservable::DATA>();
        t.timer += t.length / 15;
    } else {
        Timer& t = mFreezeDelaySub->get<TimerObservable::DATA>();
        t.timer -= t.length / 50;
    }
}

Number TimeWizard::calcFreezeEffect() {
    TimeWizard::Params params;
    return params[TimeWizard::Param::FreezeBaseEffect].get() *
           params[TimeWizard::Param::FreezeUp].get();
}

Number TimeWizard::calcSpeedEffect() {
    TimeWizard::Params params;
    Number effect = 1;
    if (params[Param::TimeWizActive].get() ||
        params[Param::TimeWizFrozen].get()) {
        effect = params[TimeWizard::Param::SpeedBaseEffect].get() +
                 (params[TimeWizard::Param::SpeedUp].get() *
                  params[TimeWizard::Param::SpeedUpUp].get());
    }
    return effect;
}

Number TimeWizard::calcCost() {
    TimeWizard::Params params;
    Number effect = params[TimeWizard::Param::SpeedEffect].get();
    Number result = min(effect - 1, .5) / 50;
    effect -= 1.5;
    if (effect > 0) {
        result += effect / 50;
    }
    result *= params[TimeWizard::Param::SpeedUpCostUp].get();
    return result;
}

Number TimeWizard::calcClockSpeed() {
    TimeWizard::Params params;
    return (params[TimeWizard::Param::FreezeEffect].get() /
            params[TimeWizard::Param::FreezeBaseEffect].get()) ^
           2;
}

void TimeWizard::updateImg() {
    Rect imgR = mImg.getRect();
    imgR.setPos(mPos->rect.cX(), mPos->rect.cY(), Rect::Align::CENTER);
    bool frozen = ParameterSystem::Param(Param::TimeWizFrozen).get();
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
