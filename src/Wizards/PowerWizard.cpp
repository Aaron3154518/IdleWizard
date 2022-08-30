#include "PowerWizard.h"

// PowerWizard
const AnimationData PowerWizard::IMG{"res/wizards/power_wizard_ss.png", 8, 150};

const std::string PowerWizard::POWER_UP_IMG =
    "res/upgrades/power_fireball_upgrade.png";

void PowerWizard::setDefaults() {
    using WizardSystem::ResetTier;

    ParameterSystem::Params<POWER_WIZARD> params;

    params[PowerWizardParams::BasePower]->init(5);
    params[PowerWizardParams::BaseSpeed]->init(.25);
    params[PowerWizardParams::Duration]->init(1000);

    params[PowerWizardParams::PowerUpLvl]->init(ResetTier::T1);
}

PowerWizard::PowerWizard() : WizardBase(POWER_WIZARD) {}

void PowerWizard::init() {
    mImg.set(IMG).setDest(IMG_RECT);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);

    WizardBase::init();
}
void PowerWizard::setSubscriptions() {
    mFireballTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            [this](Timer& t) { return onTimer(t); }, Timer(1000));
    mFreezeSub = TimeSystem::GetFreezeObservable()->subscribe(
        [this](TimeSystem::FreezeType t) { onFreeze(t); },
        [this](TimeSystem::FreezeType t) { onUnfreeze(t); });
    mAnimTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            mImg.nextFrame();
            WizardSystem::GetWizardImageObservable()->next(mId, mImg);
            return true;
        },
        IMG);
    attachSubToVisibility(mFireballTimerSub);
    attachSubToVisibility(mFreezeSub);
}
void PowerWizard::setUpgrades() {
    ParameterSystem::Params<POWER_WIZARD> params;
    ParameterSystem::States states;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(mId);
    dUp->setDescription("Power");
    dUp->setEffect(params[PowerWizardParams::Power],
                   Upgrade::Defaults::MultiplicativeEffect);
    mPowerDisplay = mUpgrades->subscribe(dUp);

    // Power upgrade
    UpgradePtr up =
        std::make_shared<Upgrade>(params[PowerWizardParams::PowerUpLvl], 15);
    up->setImage(POWER_UP_IMG);
    up->setDescription("Increase power effect by *1.15");
    up->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                params[PowerWizardParams::PowerUpCost],
                [](const Number& lvl) { return 125 * (1.5 ^ lvl); });
    up->setEffect(
        params[PowerWizardParams::PowerUp],
        [](const Number& lvl) { return 1.15 ^ lvl; },
        Upgrade::Defaults::MultiplicativeEffect);
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
                               [id](const PowerWizFireballPtr& ball) {
                                   return ball->getTargetId() == id;
                               });
                break;
        }
    }
}

void PowerWizard::onReset(WizardSystem::ResetTier tier) {
    mFireballs.clear();

    if (mFireballTimerSub) {
        mFireballTimerSub->get<TimerObservable::DATA>().reset();
    }
}

bool PowerWizard::onTimer(Timer& timer) {
    shootFireball();
    return true;
}

void PowerWizard::onFreeze(TimeSystem::FreezeType type) {}
void PowerWizard::onUnfreeze(TimeSystem::FreezeType type) {
    switch (type) {
        case TimeSystem::FreezeType::TIME_WIZARD:
            if (mFreezeFireball) {
                Number freezeEffect = ParameterSystem::Param<TIME_WIZARD>(
                                          TimeWizardParams::FreezeEffect)
                                          .get();
                mFreezeFireball->applyTimeEffect(freezeEffect);
                mFireballs.push_back(std::move(mFreezeFireball));
            }
            break;
    }
}

void PowerWizard::shootFireball() {
    ParameterSystem::Params<POWER_WIZARD> params;
    Number power = params[PowerWizardParams::Power].get();
    Number fireRing = params[PowerWizardParams::FireRingEffect].get();
    Number duration = params[PowerWizardParams::Duration].get();

    WizardId target = getTarget();

    bool frozen = TimeSystem::Frozen(TimeSystem::FreezeType::TIME_WIZARD);
    auto data = newFireballData(target);
    if (!frozen) {
        mFireballs.push_back(std::move(ComponentFactory<PowerWizFireball>::New(
            SDL_FPoint{mPos->rect.cX(), mPos->rect.cY()}, target, data)));
    } else {
        if (!mFreezeFireball) {
            mFreezeFireball = std::move(ComponentFactory<PowerWizFireball>::New(
                SDL_FPoint{mPos->rect.cX(), mPos->rect.cY()}, target, data));
        } else {
            mFreezeFireball->addFireball(data);
        }
    }
}

void PowerWizard::shootFireball(SDL_FPoint target) {
    size_t size = mFireballs.size();
    shootFireball();
    if (size != mFireballs.size()) {
        mFireballs.back()->launch(target);
    }
}

WizardId PowerWizard::getTarget() {
    int sum = mWizTargetCnt + mCrysTargetCnt;
    if (sum == 0) {
        mWizTargetCnt = mCrysTargetCnt = 4;
        sum = mWizTargetCnt + mCrysTargetCnt;
    }
    int num = (int)(rDist(gen) * sum);
    if (num < mWizTargetCnt) {
        mWizTargetCnt--;
        return WIZARD;
    }
    num -= mWizTargetCnt;
    if (num < mCrysTargetCnt) {
        mCrysTargetCnt--;
        return CRYSTAL;
    }
}

PowerWizFireball::Data PowerWizard::newFireballData(WizardId target) {
    ParameterSystem::Params<POWER_WIZARD> params;

    PowerWizFireball::Data data;
    switch (target) {
        case WIZARD:
            data.power = params[PowerWizardParams::Power].get();
            break;
        case CRYSTAL:
            data.power = params[PowerWizardParams::FireRingEffect].get();
            break;
    };
    data.duration = params[PowerWizardParams::Duration].get();
    data.sizeFactor = 1;
    return data;
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
    return (params[PowerWizardParams::Power].get() / 5) + 1;
}

void PowerWizard::setPos(float x, float y) {
    WizardBase::setPos(x, y);
    if (mFreezeFireball) {
        mFreezeFireball->setPos(mPos->rect.cX(), mPos->rect.cY());
    }
}