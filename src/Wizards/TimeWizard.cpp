#include "TimeWizard.h"

// TimeWizard
const std::string TimeWizard::ACTIVE_IMG = "res/wizards/time_wizard_active.png";
const std::string TimeWizard::FREEZE_IMG = "res/wizards/time_wizard_freeze.png";
const std::string TimeWizard::FREEZE_UP_IMG =
    "res/upgrades/time_freeze_upgrade.png";
const std::string TimeWizard::SPEED_UP_IMG = "res/upgrades/speed_upgrade.png";

const std::vector<bool> TimeWizard::DEFAULT_PARAMS = {
    ParameterSystem::SetDefault<TIME_WIZARD>(TimeWizardParams::SpeedBaseEffect,
                                             1.5),
    ParameterSystem::SetDefault<TIME_WIZARD>(TimeWizardParams::FreezeDelay,
                                             30000),
    ParameterSystem::SetDefault<TIME_WIZARD>(TimeWizardParams::FreezeDuration,
                                             5000),
    ParameterSystem::SetDefault<TIME_WIZARD>(TimeWizardParams::FreezeBaseEffect,
                                             1.1),
};

TimeWizard::TimeWizard() : WizardBase(TIME_WIZARD) {}

void TimeWizard::init() {
    // Freeze display
    mFreezePb.bkgrnd = TRANSPARENT;
    mFreezePb.blendMode = SDL_BLENDMODE_BLEND;

    WizardBase::init();
}
void TimeWizard::setSubscriptions() {
    mCostTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            std::bind(&TimeWizard::onCostTimer, this, std::placeholders::_1),
            Timer(50));
    attachSubToVisibility(mCostTimerSub);
}
void TimeWizard::setUpgrades() {
    ParameterSystem::Params<TIME_WIZARD> params;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(WIZ_IMGS.at(mId));
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
            tUp.setImage(mActive ? ACTIVE_IMG : WIZ_IMGS.at(mId));
            updateImg();
        },
        2);
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
    up->setEffects(Upgrade::Effects().addEffect(
        params[TimeWizardParams::FreezeUp],
        [](const Number& lvl) { return 1.02 ^ lvl; },
        Upgrade::Defaults::MultiplicativeEffect));
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
    up->setEffects(Upgrade::Effects().addEffect(
        params[TimeWizardParams::SpeedUp],
        [](const Number& lvl) { return lvl * .05; },
        Upgrade::Defaults::AdditiveEffect));
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
        {}, [this]() { return calcSpeedEffect(); }));
    mParamSubs.push_back(params[TimeWizardParams::SpeedCost].subscribeTo(
        {params[TimeWizardParams::SpeedEffect]}, {},
        [this]() { return calcCost(); }));
    mSpeedEffectSub = mParamSubs.back();
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
        calcSpeedEffect();
    }
    return true;
}

void TimeWizard::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    mFreezePb.rect = Rect(mPos->rect.x(), mPos->rect.y2(), mPos->rect.w(),
                          mPos->rect.h() / 15);
    TextureBuilder().draw(mFreezePb);
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

void TimeWizard::onResetT1() {
    WizardBase::onResetT1();

    if (mCostTimerSub) {
        mCostTimerSub->get<TimerObservable::DATA>().reset();
    }
}

bool TimeWizard::startFreeze(Timer& timer) {
    TimeSystem::Freeze(TimeSystem::FreezeType::TIME_WIZARD);
    mFreezeTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            std::bind(&TimeWizard::endFreeze, this, std::placeholders::_1),
            [this](Time dt, Timer& timer) {
                mFreezePb.set(1 - timer.getPercent());
            },
            Timer(ParameterSystem::Param<TIME_WIZARD>(
                      TimeWizardParams::FreezeDuration)
                      .get()
                      .toFloat()));
    mFreezePb.color = CYAN;
    updateImg();
    return false;
}

bool TimeWizard::endFreeze(Timer& timer) {
    TimeSystem::Unfreeze(TimeSystem::FreezeType::TIME_WIZARD);
    startFreezeCycle();
    return false;
}

void TimeWizard::startFreezeCycle() {
    mFreezeDelaySub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            std::bind(&TimeWizard::startFreeze, this, std::placeholders::_1),
            [this](Time dt, Timer& timer) {
                mFreezePb.set(timer.getPercent());
            },
            Timer(ParameterSystem::Param<TIME_WIZARD>(
                      TimeWizardParams::FreezeDelay)
                      .get()
                      .toFloat()));
    mFreezePb.color = BLUE;
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
    if (mActive && mCanAfford) {
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

void TimeWizard::updateImg() {
    setImage(TimeSystem::Frozen(TimeSystem::FreezeType::TIME_WIZARD)
                 ? FREEZE_IMG
             : mActive ? ACTIVE_IMG
                       : WIZ_IMGS.at(mId));
}
