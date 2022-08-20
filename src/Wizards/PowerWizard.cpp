#include "PowerWizard.h"

// PowerWizard
const std::string PowerWizard::FIREBALL_IMG = "res/projectiles/fireball2.png";
const std::string PowerWizard::POWER_UP_IMG =
    "res/upgrades/power_fireball_upgrade.png";

PowerWizard::PowerWizard() : WizardBase(POWER_WIZARD) {
    ParameterSystem::Params<POWER_WIZARD> params;
    params.set(PowerWizardParams::BasePower, 5);
    params.set(PowerWizardParams::BaseSpeed, .25);
    params.set(PowerWizardParams::FireRingEffect, 1);
    params.set(PowerWizardParams::Duration, 1000);
}

void PowerWizard::init() {
    WizardBase::init();

    mFireballTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            std::bind(&PowerWizard::onTimer, this, std::placeholders::_1),
            Timer(1000));
    mFreezeSub = TimeSystem::GetFreezeObservable()->subscribe(
        std::bind(&PowerWizard::onFreeze, this, std::placeholders::_1),
        std::bind(&PowerWizard::onUnfreeze, this, std::placeholders::_1));
    attachSubToVisibility(mFireballTimerSub);
    attachSubToVisibility(mFreezeSub);

    // Power Display
    UpgradePtr up = std::make_shared<Upgrade>();
    up->setMaxLevel(0)
        .setEffectSource(
            ParameterSystem::Param<POWER_WIZARD>(PowerWizardParams::Power),
            Upgrade::Defaults::MultiplicativeEffect)
        .setImg(WIZ_IMGS.at(mId))
        .setDescription("Power");
    mPowerDisplay = mUpgrades->subscribe(up);

    // Power upgrade
    up = std::make_shared<Upgrade>();
    up->setMaxLevel(15)
        .setEffectSource(
            ParameterSystem::Param<POWER_WIZARD>(PowerWizardParams::PowerUp),
            Upgrade::Defaults::MultiplicativeEffect)
        .setCostSource(ParameterSystem::Param<POWER_WIZARD>(
            PowerWizardParams::PowerUpCost))
        .setMoneySource(Upgrade::Defaults::CRYSTAL_MAGIC)
        .setImg(POWER_UP_IMG)
        .setDescription("Increase power effect by *1.1");
    mPowerUp = mUpgrades->subscribe(
        [this](UpgradePtr u) {
            ParameterSystem::Param<POWER_WIZARD>(PowerWizardParams::PowerUp)
                .set(Number(1.1) ^ u->getLevel());
            u->getCostSource()->set(125 * (Number(1.5) ^ u->getLevel()));
        },
        up);

    mParamSubs.push_back(
        ParameterSystem::ParamMap<POWER_WIZARD>({PowerWizardParams::BasePower,
                                                 PowerWizardParams::PowerUp,
                                                 PowerWizardParams::Speed})
            .subscribe(std::bind(&PowerWizard::calcPower, this)));
    mParamSubs.push_back(
        ParameterSystem::ParamMap<POWER_WIZARD, TIME_WIZARD>(
            {PowerWizardParams::BaseSpeed}, {TimeWizardParams::SpeedEffect})
            .subscribe(std::bind(&PowerWizard::calcSpeed, this)));
    mParamSubs.push_back(
        ParameterSystem::Param<POWER_WIZARD>(PowerWizardParams::Power)
            .subscribe(std::bind(&PowerWizard::calcFireRingEffect, this)));
    mParamSubs.push_back(
        ParameterSystem::Param<POWER_WIZARD>(PowerWizardParams::Speed)
            .subscribe(std::bind(&PowerWizard::calcTimer, this)));
}

void PowerWizard::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    for (auto it = mFireballs.begin(); it != mFireballs.end(); ++it) {
        if ((*it)->dead()) {
            it = mFireballs.erase(it);
            if (it == mFireballs.end()) {
                break;
            }
        }
    }
}

void PowerWizard::onHide(WizardId id, bool hide) {
    WizardBase::onHide(id, hide);
    if (hide) {
        switch (id) {
            case POWER_WIZARD:
                mFireballs.clear();
                break;
            default:
                std::remove_if(mFireballs.begin(), mFireballs.end(),
                               [id](const FireballPtr& ball) {
                                   return ball->getTargetId() == id;
                               });
                break;
        }
    }
}

bool PowerWizard::onTimer(Timer& timer) {
    shootFireball();
    return true;
}

void PowerWizard::onFreeze(TimeSystem::FreezeType type) {
    switch (type) {
        case TimeSystem::FreezeType::TIME_WIZARD:
            mFireballFreezeCnt = 0;
            break;
    };
}
void PowerWizard::onUnfreeze(TimeSystem::FreezeType type) {
    switch (type) {
        case TimeSystem::FreezeType::TIME_WIZARD:
            if (mFireballFreezeCnt > 0) {
                Number freezeEffect = ParameterSystem::Param<TIME_WIZARD>(
                                          TimeWizardParams::FreezeEffect)
                                          .get();
                FireballPtr& fireball = mFireballs.back();
                fireball->getValue(PowerWizardParams::Duration) ^= freezeEffect;
            }
            break;
    }
}

void PowerWizard::shootFireball() {
    ParameterSystem::Params<POWER_WIZARD> params;
    Number power = params.get(PowerWizardParams::Power);
    Number fireRing = params.get(PowerWizardParams::FireRingEffect);
    Number duration = params.get(PowerWizardParams::Duration);

    bool frozen = TimeSystem::Frozen(TimeSystem::FreezeType::TIME_WIZARD);
    if (!frozen || mFireballFreezeCnt == 0) {
        WizardId target = rDist(gen) < .5 ? WIZARD : CRYSTAL;
        mFireballs.push_back(std::move(ComponentFactory<Fireball>::New(
            SDL_FPoint{mPos->rect.cX(), mPos->rect.cY()}, mId, target,
            FIREBALL_IMG,
            NumberMap{
                {PowerWizardParams::Power, target == WIZARD ? power : fireRing},
                {PowerWizardParams::Duration, duration}})));
    }
    if (frozen) {
        FireballPtr& fireball = mFireballs.back();
        if (++mFireballFreezeCnt > 1) {
            switch (fireball->getTargetId()) {
                case WIZARD:
                    fireball->getValue(PowerWizardParams::Duration) +=
                        duration / sqrt(mFireballFreezeCnt);
                    break;
                case CRYSTAL:
                    fireball->getValue(PowerWizardParams::Power) +=
                        fireRing / sqrt(mFireballFreezeCnt) / 10;
                    break;
            }
        }
        fireball->setSize(fmin(pow(mFireballFreezeCnt, 1.0 / 3.0), 10));
    }
}

void PowerWizard::shootFireball(SDL_FPoint target) {
    size_t size = mFireballs.size();
    shootFireball();
    if (size != mFireballs.size()) {
        mFireballs.back()->launch(target);
    }
}

void PowerWizard::calcPower() {
    ParameterSystem::Params<POWER_WIZARD> params;
    Number power = params.get(PowerWizardParams::BasePower) *
                   params.get(PowerWizardParams::PowerUp) *
                   max(1, params.get(PowerWizardParams::Speed) * 16 / 1000);
    params.set(PowerWizardParams::Power, power);
}

void PowerWizard::calcSpeed() {
    ParameterSystem::Params<POWER_WIZARD> params;
    Number timeEffect =
        ParameterSystem::Param<TIME_WIZARD>(TimeWizardParams::SpeedEffect)
            .get();
    Number speed = params.get(PowerWizardParams::BaseSpeed) * timeEffect;
    params.set(PowerWizardParams::Speed, speed);
}

void PowerWizard::calcTimer() {
    Timer& timer = mFireballTimerSub->get<TimerObservable::DATA>();
    float div = ParameterSystem::Param<POWER_WIZARD>(PowerWizardParams::Speed)
                    .get()
                    .toFloat();
    if (div <= 0) {
        div = 1;
    }
    timer.length = fmax(1000 / div, 16);
}

void PowerWizard::calcFireRingEffect() {
    ParameterSystem::Params<POWER_WIZARD> params;
    Number effect = (params.get(PowerWizardParams::Power) / 10) + 1;
    params.set(PowerWizardParams::FireRingEffect, effect);
}

void PowerWizard::setPos(float x, float y) {
    WizardBase::setPos(x, y);
    if (TimeSystem::Frozen(TimeSystem::FreezeType::TIME_WIZARD) &&
        mFireballFreezeCnt > 0) {
        mFireballs.back()->setPos(mPos->rect.cX(), mPos->rect.cY());
    }
}
