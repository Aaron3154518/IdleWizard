#include "TimeWizard.h"

// TimeWizard
const std::string TimeWizard::ACTIVE_IMG = "res/wizards/time_wizard_active.png";
const std::string TimeWizard::FREEZE_IMG = "res/wizards/time_wizard_freeze.png";
const std::string TimeWizard::FREEZE_UP_IMG =
    "res/upgrades/time_freeze_upgrade.png";
const std::string TimeWizard::SPEED_UP_IMG = "res/upgrades/speed_upgrade.png";

TimeWizard::TimeWizard() : WizardBase(TIME_WIZARD) {}

void TimeWizard::init() {
    // Freeze display
    mFreezePb.bkgrnd = TRANSPARENT;
    mFreezePb.blendMode = SDL_BLENDMODE_BLEND;

    WizardBase::init();
}
void TimeWizard::setDefaultValues() {
    ParameterSystem::Params<TIME_WIZARD> params;
    params.set(TimeWizardParams::SpeedBaseEffect, 1.5);
    params.set(TimeWizardParams::FreezeDelay, 30000);
    params.set(TimeWizardParams::FreezeDuration, 5000);
    params.set(TimeWizardParams::FreezeBaseEffect, 1.1);
}
void TimeWizard::setSubscriptions() {
    WizardBase::setSubscriptions();
    mCostTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            std::bind(&TimeWizard::onCostTimer, this, std::placeholders::_1),
            Timer(50));
    startFreezeCycle();
    attachSubToVisibility(mCostTimerSub);
    attachSubToVisibility(mFreezeDelaySub);
    attachSubToVisibility(mFreezeTimerSub);
}
void TimeWizard::setUpgrades() {
    // Power Display
    UpgradePtr up = std::make_shared<Upgrade>();
    up->setMaxLevel(0)
        .setEffectSource(
            ParameterSystem::ParamMap<TIME_WIZARD>(
                {TimeWizardParams::FreezeDelay,
                 TimeWizardParams::FreezeDuration,
                 TimeWizardParams::FreezeEffect}),
            []() {
                ParameterSystem::Params<TIME_WIZARD> params;
                std::stringstream ss;
                ss << "Cooldown: "
                   << params.get(TimeWizardParams::FreezeDelay) / 1000
                   << "s\nDuration: "
                   << params.get(TimeWizardParams::FreezeDuration) / 1000
                   << "s\nUnfreeze Effect: Power ^ "
                   << params.get(TimeWizardParams::FreezeEffect);
                return ss.str();
            })
        .setImg(WIZ_IMGS.at(mId))
        .setDescription("Speed multiplier");
    mEffectDisplay = mUpgrades->subscribe(up);

    // Active toggle
    up = std::make_shared<Upgrade>();
    up->setMaxLevel(-1)
        .setImg(WIZ_IMGS.at(mId))
        .setDescription(
            "Consume magic for a fire rate multiplier to all Wizards")
        .setEffectSource(
            ParameterSystem::ParamMap<TIME_WIZARD>(
                {TimeWizardParams::SpeedCost, TimeWizardParams::SpeedEffect}),
            []() {
                ParameterSystem::Params<TIME_WIZARD> params;
                std::stringstream ss;
                ss << params.get(TimeWizardParams::SpeedEffect) << "x\n-$"
                   << params.get(TimeWizardParams::SpeedCost) << "/s";
                return ss.str();
            });
    mActiveUp = mUpgrades->subscribe(
        [this](UpgradePtr u) {
            mActive = u->getLevel() % 2 != 0;
            calcSpeedEffect();
            u->setLevel(u->getLevel() % 2)
                .setImg(mActive ? ACTIVE_IMG : WIZ_IMGS.at(mId));
            updateImg();
        },
        up);

    // Freeze upgrade
    up = std::make_shared<Upgrade>();
    up->setMaxLevel(8)
        .setImg(FREEZE_UP_IMG)
        .setDescription("Multiply unfreeze boost exponent by 1.2")
        .setEffectSource(
            ParameterSystem::Param<TIME_WIZARD>(TimeWizardParams::FreezeUp),
            Upgrade::Defaults::MultiplicativeEffect)
        .setCostSource(
            ParameterSystem::Param<TIME_WIZARD>(TimeWizardParams::FreezeUpCost))
        .setMoneySource(Upgrade::Defaults::CRYSTAL_MAGIC);
    mFreezeUp = mUpgrades->subscribe(
        [this](UpgradePtr u) {
            ParameterSystem::Param<TIME_WIZARD>(TimeWizardParams::FreezeUp)
                .set(Number(1.02) ^ u->getLevel());
            u->getCostSource()->set(150 * (Number(1.6) ^ u->getLevel()));
        },
        up);

    // Speed upgrade
    up = std::make_shared<Upgrade>();
    up->setMaxLevel(10)
        .setImg(SPEED_UP_IMG)
        .setDescription(
            "Increase speed boost multiplier by +.05\nThis will also increase "
            "the magic cost!")
        .setEffectSource(
            ParameterSystem::Param<TIME_WIZARD>(TimeWizardParams::SpeedUp),
            Upgrade::Defaults::AdditiveEffect)
        .setCostSource(
            ParameterSystem::Param<TIME_WIZARD>(TimeWizardParams::SpeedUpCost))
        .setMoneySource(Upgrade::Defaults::CRYSTAL_MAGIC);
    mSpeedUp = mUpgrades->subscribe(
        [this](UpgradePtr u) {
            ParameterSystem::Param<TIME_WIZARD>(TimeWizardParams::SpeedUp)
                .set(u->getLevel() * .05);
            u->getCostSource()->set(Number(250) ^
                                    (1 + (float)u->getLevel() / 12));
        },
        up);
}
void TimeWizard::setFormulas() {
    mParamSubs.push_back(
        ParameterSystem::ParamMap<TIME_WIZARD>(
            {TimeWizardParams::FreezeBaseEffect, TimeWizardParams::FreezeUp})
            .subscribe(std::bind(&TimeWizard::calcFreezeEffect, this)));
    mParamSubs.push_back(
        ParameterSystem::ParamMap<TIME_WIZARD>(
            {TimeWizardParams::SpeedBaseEffect, TimeWizardParams::SpeedUp})
            .subscribe(std::bind(&TimeWizard::calcSpeedEffect, this)));
    mParamSubs.push_back(
        ParameterSystem::Param<TIME_WIZARD>(TimeWizardParams::SpeedEffect)
            .subscribe(std::bind(&TimeWizard::calcCost, this)));
}

bool TimeWizard::onCostTimer(Timer& timer) {
    bool prevAfford = mCanAfford;
    mCanAfford = false;
    if (mActive) {
        ParameterSystem::Param<TIME_WIZARD> speedParam(
            TimeWizardParams::SpeedCost);
        ParameterSystem::Param<CRYSTAL> magicParam(CrystalParams::Magic);
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
    if (id == TIME_WIZARD) {
        if (hide) {
            TimeSystem::Unfreeze(TimeSystem::FreezeType::TIME_WIZARD);
            mFreezeDelaySub.reset();
            mFreezeTimerSub.reset();
            mActive = false;
            UpgradeList::Get(mActiveUp)->updateEffect();
        } else {
            startFreezeCycle();
        }
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

void TimeWizard::calcFreezeEffect() {
    ParameterSystem::Params<TIME_WIZARD> params;
    Number effect = params.get(TimeWizardParams::FreezeBaseEffect) *
                    params.get(TimeWizardParams::FreezeUp);
    params.set(TimeWizardParams::FreezeEffect, effect);
}

void TimeWizard::calcSpeedEffect() {
    ParameterSystem::Params<TIME_WIZARD> params;
    Number effect = 1;
    if (mActive && mCanAfford) {
        effect = params.get(TimeWizardParams::SpeedBaseEffect) +
                 params.get(TimeWizardParams::SpeedUp);
    }
    params.set(TimeWizardParams::SpeedEffect, effect);
}

void TimeWizard::calcCost() {
    ParameterSystem::Params<TIME_WIZARD> params;
    Number effect = params.get(TimeWizardParams::SpeedEffect);
    Number cost = 2 * (effect - 1) * (10 ^ (effect ^ (effect / 3)));
    params.set(TimeWizardParams::SpeedCost, cost);
}

void TimeWizard::updateImg() {
    setImage(TimeSystem::Frozen(TimeSystem::FreezeType::TIME_WIZARD)
                 ? FREEZE_IMG
             : mActive ? ACTIVE_IMG
                       : WIZ_IMGS.at(mId));
}
