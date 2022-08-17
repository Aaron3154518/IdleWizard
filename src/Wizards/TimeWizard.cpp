#include "TimeWizard.h"

// TimeWizard
const std::string TimeWizard::ACTIVE_IMG = "res/wizards/time_wizard_active.png";
const std::string TimeWizard::FREEZE_IMG = "res/wizards/time_wizard_freeze.png";
const std::string TimeWizard::FREEZE_UP_IMG =
    "res/upgrades/time_freeze_upgrade.png";

TimeWizard::TimeWizard() : WizardBase(TIME_WIZARD) {
    auto params = ParameterSystem::Get();
    params->set<TIME_WIZARD>(TimeWizardParams::SpeedBaseEffect, 1.5);
    params->set<TIME_WIZARD>(TimeWizardParams::SpeedUp, 0);
    params->set<TIME_WIZARD>(TimeWizardParams::FreezeDelay, 30000);
    params->set<TIME_WIZARD>(TimeWizardParams::FreezeDuration, 5000);
    params->set<TIME_WIZARD>(TimeWizardParams::FreezeEffect, 1.1);
}

void TimeWizard::init() {
    WizardBase::init();

    // Freeze display
    mFreezePb.bkgrnd = TRANSPARENT;
    mFreezePb.blendMode = SDL_BLENDMODE_BLEND;

    mCostTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            std::bind(&TimeWizard::onCostTimer, this, std::placeholders::_1),
            Timer(50));
    startFreezeCycle();
    attachSubToVisibility(mCostTimerSub);
    attachSubToVisibility(mFreezeDelaySub);
    attachSubToVisibility(mFreezeTimerSub);

    // Power Display
    UpgradePtr up = std::make_shared<Upgrade>();
    up->setMaxLevel(0)
        .setEffectSource<Keys<TIME_WIZARD, TimeWizardParams::FreezeDelay,
                              TimeWizardParams::FreezeDuration,
                              TimeWizardParams::FreezeEffect>>([]() {
            auto params = ParameterSystem::Get();
            std::stringstream ss;
            ss << "Cooldown: "
               << params->get<TIME_WIZARD>(TimeWizardParams::FreezeDelay) / 1000
               << "s\nDuration: "
               << params->get<TIME_WIZARD>(TimeWizardParams::FreezeDuration) /
                      1000
               << "s\nUnfreeze Effect: Power ^ "
               << params->get<TIME_WIZARD>(TimeWizardParams::FreezeEffect);
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
        .setEffectSource<Keys<TIME_WIZARD, TimeWizardParams::SpeedCost,
                              TimeWizardParams::SpeedEffect>>([this]() {
            auto params = ParameterSystem::Get();
            std::stringstream ss;
            ss << params->get<TIME_WIZARD>(TimeWizardParams::SpeedEffect)
               << "x\n-$"
               << params->get<TIME_WIZARD>(TimeWizardParams::SpeedCost) << "/s";
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

    // Speed upgrade
    up = std::make_shared<Upgrade>();
    up->setMaxLevel(10)
        .setImg("")
        .setDescription(
            "Increase speed boost multiplier by +.05\nThis will also increase "
            "the magic cost!")
        .setEffectSource<TIME_WIZARD>(TimeWizardParams::SpeedUp,
                                      Upgrade::Defaults::AdditiveEffect)
        .setCostSource<TIME_WIZARD>(TimeWizardParams::SpeedUpCost)
        .setMoneySource<CRYSTAL>(CrystalParams::Magic);
    mSpeedUp = mUpgrades->subscribe(
        [this](UpgradePtr u) {
            u->getEffectSrc().set(u->getLevel() * .05);
            u->getCostSrc().set(Number(250) ^ (1 + (float)u->getLevel() / 15));
        },
        up);

    auto params = ParameterSystem::Get();
    mParamSubs.push_back(
        params->subscribe<Keys<TIME_WIZARD, TimeWizardParams::SpeedBaseEffect,
                               TimeWizardParams::SpeedUp>>(
            std::bind(&TimeWizard::calcSpeedEffect, this)));
    mParamSubs.push_back(params->subscribe<TIME_WIZARD>(
        TimeWizardParams::SpeedEffect, std::bind(&TimeWizard::calcCost, this)));
}

bool TimeWizard::onCostTimer(Timer& timer) {
    auto params = ParameterSystem::Get();
    bool prevAfford = mCanAfford;
    mCanAfford = false;
    if (mActive) {
        Number cost = params->get<TIME_WIZARD>(TimeWizardParams::SpeedCost) *
                      timer.length / 1000;
        Number money = params->get<CRYSTAL>(CrystalParams::Magic);
        if (cost <= money) {
            params->set<CRYSTAL>(CrystalParams::Magic, money - cost);
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
            Timer(ParameterSystem::Get()
                      ->get<TIME_WIZARD>(TimeWizardParams::FreezeDuration)
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
            Timer(ParameterSystem::Get()
                      ->get<TIME_WIZARD>(TimeWizardParams::FreezeDelay)
                      .toFloat()));
    mFreezePb.color = BLUE;
    updateImg();
}

void TimeWizard::calcSpeedEffect() {
    auto params = ParameterSystem::Get();
    Number effect = 1;
    if (mActive && mCanAfford) {
        effect = params->get<TIME_WIZARD>(TimeWizardParams::SpeedBaseEffect) +
                 params->get<TIME_WIZARD>(TimeWizardParams::SpeedUp);
    }
    params->set<TIME_WIZARD>(TimeWizardParams::SpeedEffect, effect);
}

void TimeWizard::calcCost() {
    auto params = ParameterSystem::Get();
    Number effect = params->get<TIME_WIZARD>(TimeWizardParams::SpeedEffect);
    Number cost = 2 * (effect - 1) * (10 ^ (effect ^ (effect / 3)));
    params->set<TIME_WIZARD>(TimeWizardParams::SpeedCost, cost);
}

void TimeWizard::updateImg() {
    setImage(TimeSystem::Frozen(TimeSystem::FreezeType::TIME_WIZARD)
                 ? FREEZE_IMG
             : mActive ? ACTIVE_IMG
                       : WIZ_IMGS.at(mId));
}
