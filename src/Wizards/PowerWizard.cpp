#include "PowerWizard.h"

// PowerWizard
PowerWizard::PowerWizard() : WizardBase(POWER_WIZARD) {}

void PowerWizard::init() {
    mImg.set(PowerWizardDefs::IMG).setDest(IMG_RECT);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);

    mTargets = {{WIZARD, 0}, {CRYSTAL, 0}};

    WizardBase::init();
}
void PowerWizard::setSubscriptions() {
    mFireballTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            [this](Timer& t) { return onTimer(t); }, Timer(1000));
    mAnimTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            mImg.nextFrame();
            WizardSystem::GetWizardImageObservable()->next(mId, mImg);
            return true;
        },
        PowerWizardDefs::IMG);
    mT1ResetSub = WizardSystem::GetWizardEventObservable()->subscribe(
        [this]() { onT1Reset(); }, WizardSystem::Event::ResetT1);
    mTargetHideSub = WizardSystem::GetHideObservable()->subscribeToAll(
        [this](WizardId id, bool hide) { onTargetHide(id, hide); });
    attachSubToVisibility(mFireballTimerSub);
}
void PowerWizard::setUpgrades() {
    ParameterSystem::Params<POWER_WIZARD> params;
    ParameterSystem::States states;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(mId);
    dUp->setDescription({"Power"});
    dUp->setEffects(
        {params[PowerWizardParams::Power], params[PowerWizardParams::FBSpeed],
         params[PowerWizardParams::FBSpeedEffect]},
        {}, []() -> TextUpdateData {
            ParameterSystem::Params<POWER_WIZARD> params;
            std::stringstream ss;
            std::vector<RenderDataWPtr> imgs;
            ss << "Power: "
               << Upgrade::Defaults::MultiplicativeEffect(
                      params[PowerWizardParams::Power].get())
                      .text
               << "\n{i} Speed: "
               << Upgrade::Defaults::MultiplicativeEffect(
                      params[PowerWizardParams::FBSpeed].get())
                      .text
               << ", {b}Power : "
               << Upgrade::Defaults::MultiplicativeEffect(
                      params[PowerWizardParams::FBSpeedEffect].get())
                      .text;
            imgs.push_back(PowerWizFireball::GetIcon());
            return {ss.str(), imgs};
        });
    mPowerDisplay = mUpgrades->subscribe(dUp);

    // Power upgrade
    UpgradePtr up =
        std::make_shared<Upgrade>(params[PowerWizardParams::PowerUpLvl], 15);
    up->setImage(PowerWizardDefs::POWER_UP_IMG);
    up->setDescription(
        {"Increase {i} power by *1.15", {PowerWizFireball::GetIcon()}});
    up->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                params[PowerWizardParams::PowerUpCost],
                [](const Number& lvl) { return 125 * (1.5 ^ lvl); });
    up->setEffect(
        params[PowerWizardParams::PowerUp],
        [](const Number& lvl) { return 1.15 ^ lvl; },
        Upgrade::Defaults::MultiplicativeEffect);
    mPowerUp = mUpgrades->subscribe(up);

    // Time warp upgrade
    up = std::make_shared<Upgrade>(params[PowerWizardParams::TimeWarpUpLvl], 6);
    up->setImage(PowerWizardDefs::TIME_WARP_UP_IMG);
    up->setDescription(
        {"Unlocks time warp - {i} boosts {i}, speeding up all "
         "{i}\nSped up fireballs gain power based on {i} "
         "effect",
         {PowerWizardDefs::GetIcon(), TimeWizardDefs::GetIcon(),
          WizardFireball::GetIcon(), PowerWizFireball::GetIcon()}});
    up->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                params[PowerWizardParams::TimeWarpUpCost],
                [](const Number& lvl) { return Number(8, 3) * (3 ^ lvl); });
    up->setEffects(
        {{params[PowerWizardParams::TimeWarpUp],
          [](const Number& lvl) { return lvl == 0 ? 1 : .8 + lvl / 5; }}},
        {{states[State::TimeWarpEnabled],
          [](const Number& lvl) { return lvl > 0; }}},
        []() -> TextUpdateData {
            std::stringstream ss;
            ss << "*{i}^"
               << ParameterSystem::Param<POWER_WIZARD>(
                      PowerWizardParams::TimeWarpUp)
                      .get();
            return {ss.str(), {PowerWizFireball::GetIcon()}};
        });
    mTimeWarpUp = mUpgrades->subscribe(up);
}
void PowerWizard::setParamTriggers() {
    ParameterSystem::Params<POWER_WIZARD> params;
    ParameterSystem::Params<TIME_WIZARD> timeParams;
    ParameterSystem::States states;

    mParamSubs.push_back(params[PowerWizardParams::Duration].subscribeTo(
        ParameterSystem::Param<WIZARD>(WizardParams::BaseSpeed),
        [](const Number& val) { return val == 0 ? 1000 : (1000 / val); }));

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

    mParamSubs.push_back(params[PowerWizardParams::FBSpeed].subscribeTo(
        {params[PowerWizardParams::BaseFBSpeed],
         timeParams[TimeWizardParams::FBSpeedUp]},
        {}, [this]() { return calcFBSpeed(); }));

    mParamSubs.push_back(params[PowerWizardParams::FBSpeedEffect].subscribeTo(
        {params[PowerWizardParams::FBSpeed]}, {},
        [this]() { return calcFBSpeedEffect(); }));

    mParamSubs.push_back(states[State::TimeWizFrozen].subscribe(
        [this](bool val) { onTimeFreeze(val); }));

    mParamSubs.push_back(
        params[PowerWizardParams::Speed].subscribe([this]() { calcTimer(); }));

    mParamSubs.push_back(
        states[State::BoughtPowerWizard].subscribe([this](bool bought) {
            WizardSystem::GetHideObservable()->next(mId, !bought);
        }));

    mParamSubs.push_back(states[State::BoughtSecondT1].subscribe(
        [this](bool bought) { mTimeWarpUp->setActive(bought); }));

    mParamSubs.push_back(
        states[State::TimeWarpEnabled].subscribe([this](bool enabled) {
            if (!enabled) {
                mTargets[TIME_WIZARD] = -1;
            } else {
                mTargets[TIME_WIZARD] = 0;
                for (auto& pair : mTargets) {
                    if (pair.second > 0) {
                        pair.second = 0;
                    }
                }
            }
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

void PowerWizard::onHide(bool hide) {
    WizardBase::onHide(hide);
    mFireballs.clear();
}

void PowerWizard::onTargetHide(WizardId id, bool hide) {
    if (hide) {
        std::remove_if(mFireballs.begin(), mFireballs.end(),
                       [id](const PowerWizFireballPtr& ball) {
                           return ball->getTargetId() == id;
                       });
    }
}

void PowerWizard::onT1Reset() {
    mFireballs.clear();

    if (mFireballTimerSub) {
        mFireballTimerSub->get<TimerObservable::DATA>().reset();
    }
}

bool PowerWizard::onTimer(Timer& timer) {
    shootFireball();
    return true;
}

void PowerWizard::onTimeFreeze(bool frozen) {
    if (!frozen && mFreezeFireball) {
        Number freezeEffect =
            ParameterSystem::Param<TIME_WIZARD>(TimeWizardParams::FreezeEffect)
                .get();
        mFreezeFireball->applyTimeEffect(freezeEffect);
        mFireballs.push_back(std::move(mFreezeFireball));
    }
}

void PowerWizard::shootFireball() {
    WizardId target = getTarget();

    auto data = newFireballData(target);
    if (!ParameterSystem::Param(State::TimeWizFrozen).get()) {
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
    const static int CNT = 2;

    int sum = 0;
    for (auto& pair : mTargets) {
        if (pair.second > 0) {
            sum += pair.second;
        }
    }

    if (sum == 0) {
        for (auto& pair : mTargets) {
            if (pair.second >= 0) {
                pair.second = CNT;
                sum += pair.second;
            }
        }
    }

    int num = (int)(rDist(gen) * sum);

    for (auto& pair : mTargets) {
        if (pair.second > 0) {
            if (num < pair.second) {
                pair.second--;
                return pair.first;
            }
            num -= pair.second;
        }
    }

    return CRYSTAL;
}

PowerWizFireball::Data PowerWizard::newFireballData(WizardId target) {
    ParameterSystem::Params<POWER_WIZARD> params;
    Number speedEffect = params[PowerWizardParams::FBSpeedEffect].get();

    PowerWizFireball::Data data;
    switch (target) {
        case WIZARD:
            data.duration = params[PowerWizardParams::Duration].get();
            data.power = params[PowerWizardParams::Power].get() * speedEffect;
            break;
        case TIME_WIZARD:
            data.power =
                (params[PowerWizardParams::Power].get() * speedEffect) ^
                params[PowerWizardParams::TimeWarpUp].get();
            break;
        case CRYSTAL:
            data.duration = params[PowerWizardParams::Duration].get() * 2;
            data.power =
                params[PowerWizardParams::FireRingEffect].get() * speedEffect;
            break;
    };
    data.sizeFactor = 1;
    data.speed = params[PowerWizardParams::FBSpeed].get().toFloat();
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

Number PowerWizard::calcFBSpeed() {
    ParameterSystem::Params<POWER_WIZARD> params;
    ParameterSystem::Params<TIME_WIZARD> timeParams;

    return params[PowerWizardParams::BaseFBSpeed].get() *
           timeParams[TimeWizardParams::FBSpeedUp].get();
}

Number PowerWizard::calcFBSpeedEffect() {
    ParameterSystem::Params<POWER_WIZARD> params;
    Number fbSpeed = max(params[PowerWizardParams::FBSpeed].get(), 1);

    Number twoFbSpeed = 2 * fbSpeed;

    return fbSpeed * (twoFbSpeed ^ (twoFbSpeed - 2));
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