#include "PowerWizard.h"

// PowerWizard
const std::string PowerWizard::FIREBALL_IMG = "res/projectiles/fireball2.png";
const std::string PowerWizard::POWER_UP_IMG =
    "res/upgrades/power_fireball_upgrade.png";

void PowerWizard::setDefaults() {
    ParameterSystem::Params<POWER_WIZARD> params;

    params[PowerWizardParams::BasePower]->setDefault(5);
    params[PowerWizardParams::BaseSpeed]->setDefault(.25);
    params[PowerWizardParams::Duration]->setDefault(1000);
}

PowerWizard::PowerWizard() : WizardBase(POWER_WIZARD) {}

void PowerWizard::setSubscriptions() {
    mFireballTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            [this](Timer& t) { return onTimer(t); }, Timer(1000));
    mFreezeSub = TimeSystem::GetFreezeObservable()->subscribe(
        [this](TimeSystem::FreezeType t) { onFreeze(t); },
        [this](TimeSystem::FreezeType t) { onUnfreeze(t); });
    attachSubToVisibility(mFireballTimerSub);
    attachSubToVisibility(mFreezeSub);
}
void PowerWizard::setUpgrades() {
    ParameterSystem::Params<POWER_WIZARD> params;
    ParameterSystem::States states;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(WIZ_IMGS.at(mId));
    dUp->setDescription("Power");
    dUp->setEffect(params[PowerWizardParams::Power],
                   Upgrade::Defaults::MultiplicativeEffect);
    mPowerDisplay = mUpgrades->subscribe(dUp);

    // Power upgrade
    UpgradePtr up =
        std::make_shared<Upgrade>(params[PowerWizardParams::PowerUpLvl], 15);
    up->setImage(POWER_UP_IMG);
    up->setDescription("Increase power effect by *1.1");
    up->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                params[PowerWizardParams::PowerUpCost],
                [](const Number& lvl) { return 125 * (1.5 ^ lvl); });
    up->setEffects(Upgrade::Effects().addEffect(
        params[PowerWizardParams::PowerUp],
        [](const Number& lvl) { return 1.1 ^ lvl; },
        Upgrade::Defaults::MultiplicativeEffect));
    mPowerUp = mUpgrades->subscribe(up);
}
void PowerWizard::setParamTriggers() {
    ParameterSystem::Params<POWER_WIZARD> params;
    ParameterSystem::Params<TIME_WIZARD> timeParams;
    ParameterSystem::States states;

    mParamSubs.push_back(params[PowerWizardParams::Power].subscribeTo(
        {params[PowerWizardParams::BasePower],
         params[PowerWizardParams::PowerUp], params[PowerWizardParams::Speed]},
        {}, [this]() { return calcPower(); }));

    mParamSubs.push_back(params[PowerWizardParams::Speed].subscribeTo(
        {params[PowerWizardParams::BaseSpeed],
         timeParams[TimeWizardParams::SpeedEffect]},
        {}, [this]() { return calcSpeed(); }));

    mParamSubs.push_back(params[PowerWizardParams::FireRingEffect].subscribeTo(
        {params[PowerWizardParams::Power]}, {},
        [this]() { return calcFireRingEffect(); }));

    mParamSubs.push_back(
        params[PowerWizardParams::Speed].subscribe([this]() { calcTimer(); }));

    mParamSubs.push_back(
        states[State::BoughtPowerWizard].subscribe([this](bool bought) {
            WizardSystem::GetHideObservable()->next(mId, !bought);
        }));
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

void PowerWizard::onResetT1() {
    mFireballs.clear();
    mFireballFreezeCnt = 0;

    if (mFireballTimerSub) {
        mFireballTimerSub->get<TimerObservable::DATA>().reset();
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
            if (mFireballFreezeCnt > 0 && !mFireballs.empty()) {
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
    Number power = params[PowerWizardParams::Power].get();
    Number fireRing = params[PowerWizardParams::FireRingEffect].get();
    Number duration = params[PowerWizardParams::Duration].get();

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

Number PowerWizard::calcPower() {
    ParameterSystem::Params<POWER_WIZARD> params;
    return params[PowerWizardParams::BasePower].get() *
           params[PowerWizardParams::PowerUp].get() *
           max(1, params[PowerWizardParams::Speed].get() * 16 / 1000);
}

Number PowerWizard::calcSpeed() {
    ParameterSystem::Params<POWER_WIZARD> params;
    ParameterSystem::Params<TIME_WIZARD> timeParams;
    Number timeEffect = timeParams[TimeWizardParams::SpeedEffect].get();
    return params[PowerWizardParams::BaseSpeed].get() * timeEffect;
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

Number PowerWizard::calcFireRingEffect() {
    ParameterSystem::Params<POWER_WIZARD> params;
    return (params[PowerWizardParams::Power].get() / 10) + 1;
}

void PowerWizard::setPos(float x, float y) {
    WizardBase::setPos(x, y);
    if (TimeSystem::Frozen(TimeSystem::FreezeType::TIME_WIZARD) &&
        mFireballFreezeCnt > 0) {
        mFireballs.back()->setPos(mPos->rect.cX(), mPos->rect.cY());
    }
}
