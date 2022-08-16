#include "TimeWizard.h"

// TimeWizard
const std::string TimeWizard::ACTIVE_IMG = "res/wizards/time_wizard_active.png";
const std::string TimeWizard::FREEZE_IMG = "res/wizards/time_wizard_freeze.png";
const std::string TimeWizard::FREEZE_UP_IMG =
    "res/upgrades/time_freeze_upgrade.png";

TimeWizard::TimeWizard() : WizardBase(TIME_WIZARD) {
    auto params = ParameterSystem::Get();
    params->set<TIME_WIZARD>(TimeWizardParams::SpeedPower, 1.5);
    params->set<TIME_WIZARD>(TimeWizardParams::SpeedEffect, 1);
    params->set<TIME_WIZARD>(TimeWizardParams::FreezeDelay, 30000);
    params->set<TIME_WIZARD>(TimeWizardParams::FreezeDuration, 5000);
    params->set<TIME_WIZARD>(TimeWizardParams::FreezeEffect, 1.1);
}

void TimeWizard::init() {
    WizardBase::init();

    // Freeze display
    mFreezePb.bkgrnd = TRANSPARENT;
    mFreezePb.blendMode = SDL_BLENDMODE_BLEND;

    mUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            std::bind(&TimeWizard::onUpdate, this, std::placeholders::_1));
    startFreezeCycle();
    attachSubToVisibility(mUpdateSub);
    attachSubToVisibility(mFreezeDelaySub);
    attachSubToVisibility(mFreezeTimerSub);

    // Power Display
    UpgradePtr up = std::make_shared<Upgrade>();
    up->setMaxLevel(0)
        .setEffectSource<TIME_WIZARD, TimeWizardParams::FreezeDuration>(
            [](const Number& val) {
                auto params = ParameterSystem::Get();
                std::stringstream ss;
                ss << "Cooldown: "
                   << params->get<TIME_WIZARD>(TimeWizardParams::FreezeDelay) /
                          1000
                   << "s\nDuration: "
                   << params->get<TIME_WIZARD>(
                          TimeWizardParams::FreezeDuration) /
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
        .setEffectSource<TIME_WIZARD, TimeWizardParams::SpeedCost>(
            [this](const Number& val) {
                auto params = ParameterSystem::Get();
                std::stringstream ss;
                ss << params->get<TIME_WIZARD>(TimeWizardParams::SpeedEffect)
                   << "x\n-$"
                   << (mActive ? params->get<TIME_WIZARD>(
                                     TimeWizardParams::SpeedCost)
                               : 0)
                   << "/s";
                return ss.str();
            });
    mActiveUp = mUpgrades->subscribe(
        [this](UpgradePtr u) {
            mActive = u->getLevel() % 2 != 0;
            u->setLevel(u->getLevel() % 2)
                .setImg(mActive ? ACTIVE_IMG : WIZ_IMGS.at(mId))
                .updateEffect();
            updateImg();
        },
        up);

    auto params = ParameterSystem::Get();
    mParamSubs.push_back(params->subscribe<TIME_WIZARD>(
        TimeWizardParams::SpeedEffect, std::bind(&TimeWizard::calcCost, this)));
}

void TimeWizard::onUpdate(Time dt) {
    auto params = ParameterSystem::Get();
    if (mActive) {
        Number cost =
            params->get<TIME_WIZARD>(TimeWizardParams::SpeedCost) * dt.s();
        Number money = params->get<CRYSTAL>(CrystalParams::Magic);
        Number effect = params->get<TIME_WIZARD>(TimeWizardParams::SpeedEffect);
        if (cost <= money) {
            params->set<CRYSTAL>(CrystalParams::Magic, money - cost);
            if (!mCanAfford) {
                params->set<TIME_WIZARD>(
                    TimeWizardParams::SpeedEffect,
                    params->get<TIME_WIZARD>(TimeWizardParams::SpeedPower));
            }
            mCanAfford = true;
        } else if (mCanAfford) {
            params->set<TIME_WIZARD>(TimeWizardParams::SpeedEffect, 1);
            mCanAfford = false;
        }
    } else if (mCanAfford) {
        params->set<TIME_WIZARD>(TimeWizardParams::SpeedEffect, 1);
        mCanAfford = false;
    }
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

void TimeWizard::calcCost() {
    auto params = ParameterSystem::Get();
    params->set<TIME_WIZARD>(
        TimeWizardParams::SpeedCost,
        10 ^ (params->get<TIME_WIZARD>(TimeWizardParams::SpeedPower) * .5));
}

void TimeWizard::updateImg() {
    setImage(TimeSystem::Frozen(TimeSystem::FreezeType::TIME_WIZARD)
                 ? FREEZE_IMG
             : mActive ? ACTIVE_IMG
                       : WIZ_IMGS.at(mId));
}
