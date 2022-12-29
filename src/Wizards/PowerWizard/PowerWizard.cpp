#include "PowerWizard.h"

namespace PowerWizard {
// PowerWizard
PowerWizard::PowerWizard() : WizardBase(POWER_WIZARD) {}

void PowerWizard::init() {
    mFireballs = ComponentFactory<FireballList>::New();

    mImg.set(Constants::IMG());
    mImg.setDest(IMG_RECT);
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
            mImg->nextFrame();
            WizardSystem::GetWizardImageObservable()->next(mId, mImg);
            return true;
        },
        Constants::IMG());
    mT1ResetSub = WizardSystem::GetWizardEventObservable()->subscribe(
        [this]() { onT1Reset(); }, WizardSystem::Event::ResetT1);
    mTargetHideSub = WizardSystem::GetHideObservable()->subscribeToAll(
        [this](WizardId id, bool hide) { onTargetHide(id, hide); });
    attachSubToVisibility(mFireballTimerSub);
}
void PowerWizard::setUpgrades() {
    PowerWizard::Params params;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(mId);
    dUp->setDescription({"Power"});
    dUp->setEffects(
        {params[PowerWizard::Param::Power], params[PowerWizard::Param::FBSpeed],
         params[PowerWizard::Param::FBSpeedEffect]},
        {}, []() -> TextUpdateData {
            PowerWizard::Params params;
            std::stringstream ss;
            std::vector<RenderTextureCPtr> imgs;
            ss << "Power: "
               << UpgradeDefaults::MultiplicativeEffect(
                      params[PowerWizard::Param::Power].get())
                      .text
               << "\n{i} Speed: "
               << UpgradeDefaults::MultiplicativeEffect(
                      params[PowerWizard::Param::FBSpeed].get())
                      .text
               << ", {b}Power : "
               << UpgradeDefaults::MultiplicativeEffect(
                      params[PowerWizard::Param::FBSpeedEffect].get())
                      .text;
            imgs.push_back(IconSystem::Get(Constants::FB_IMG()));
            return {ss.str(), imgs};
        });
    mPowerDisplay = mUpgrades->subscribe(dUp);

    // Power upgrade
    UpgradePtr up =
        std::make_shared<Upgrade>(params[PowerWizard::Param::PowerUpLvl], 15);
    up->setImage(Constants::POWER_UP_IMG);
    up->setDescription({"Increase {i} power by *1.15",
                        {IconSystem::Get(Constants::FB_IMG())}});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                params[PowerWizard::Param::PowerUpCost]);
    up->setEffects(params[PowerWizard::Param::PowerUp],
                   UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(UpgradeDefaults::subscribeT1UpCost(
        up->level(), params[PowerWizard::Param::PowerUpCost],
        [](const Number& lvl) { return 175 * (1.6 ^ lvl); }));
    mParamSubs.push_back(params[PowerWizard::Param::PowerUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1.15 ^ lvl; }));
    mPowerUp = mUpgrades->subscribe(up);

    // Time warp upgrade
    up = std::make_shared<Upgrade>(params[PowerWizard::Param::TimeWarpUpLvl], 6);
    up->setImage(Constants::TIME_WARP_UP_IMG);
    up->setDescription(
        {"Unlocks time warp - {i} boosts {i}, speeding up all "
         "{i}\nSped up fireballs gain power based on {i} "
         "effect",
         {IconSystem::Get(Constants::IMG()),
          IconSystem::Get(TimeWizard::Constants::IMG()),
          IconSystem::Get(Wizard::Constants::FB_IMG()),
          IconSystem::Get(Constants::FB_IMG())}});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                params[PowerWizard::Param::TimeWarpUpCost]);
    up->setEffects(
        {params[PowerWizard::Param::TimeWarpUp]},
        {states[State::TimeWarpEnabled]}, []() -> TextUpdateData {
            std::stringstream ss;
            ss << "*{i}^"
               << PowerWizard::Params::get(
                      PowerWizard::Param::TimeWarpUp)
                      .get();
            return {ss.str(), {IconSystem::Get(Constants::FB_IMG())}};
        });
    mParamSubs.push_back(params[PowerWizard::Param::TimeWarpUpCost].subscribeTo(
        up->level(),
        [](const Number& lvl) { return Number(1, 8) * (3 ^ lvl); }));
    mParamSubs.push_back(params[PowerWizard::Param::TimeWarpUp].subscribeTo(
        up->level(),
        [](const Number& lvl) { return lvl == 0 ? 1 : .8 + lvl / 5; }));
    mParamSubs.push_back(states[State::TimeWarpEnabled].subscribeTo(
        up->level(), [](const Number& lvl) { return lvl > 0; }));
    mTimeWarpUp = mUpgrades->subscribe(up);
}
void PowerWizard::setParamTriggers() {
    PowerWizard::Params params;
    TimeWizard::Params timeParams;

    mParamSubs.push_back(params[PowerWizard::Param::Duration].subscribeTo(
        Wizard::Params::get(Wizard::Param::BaseSpeed),
        [](const Number& val) { return val == 0 ? 1000 : (1000 / val); }));

    mParamSubs.push_back(params[PowerWizard::Param::Power].subscribeTo(
        {params[PowerWizard::Param::BasePower],
         params[PowerWizard::Param::PowerUp], params[PowerWizard::Param::Speed]},
        {}, [this]() { return calcPower(); }));

    mParamSubs.push_back(params[PowerWizard::Param::Speed].subscribeTo(
        {params[PowerWizard::Param::BaseSpeed],
         timeParams[TimeWizard::Param::SpeedEffect]},
        {}, [this]() { return calcSpeed(); }));

    mParamSubs.push_back(params[PowerWizard::Param::FireRingEffect].subscribeTo(
        {params[PowerWizard::Param::Power]}, {},
        [this]() { return calcFireRingEffect(); }));

    mParamSubs.push_back(params[PowerWizard::Param::FBSpeed].subscribeTo(
        {params[PowerWizard::Param::BaseFBSpeed],
         timeParams[TimeWizard::Param::FBSpeedUp]},
        {}, [this]() { return calcFBSpeed(); }));

    mParamSubs.push_back(params[PowerWizard::Param::FBSpeedEffect].subscribeTo(
        {params[PowerWizard::Param::FBSpeed]}, {},
        [this]() { return calcFBSpeedEffect(); }));

    mParamSubs.push_back(states[State::TimeWizFrozen].subscribe(
        [this](bool val) { onTimeFreeze(val); }));

    mParamSubs.push_back(
        params[PowerWizard::Param::Speed].subscribe([this]() { calcTimer(); }));

    mParamSubs.push_back(
        states[Crystal::Param::BoughtPowerWizard].subscribe([this](bool bought) {
            WizardSystem::GetHideObservable()->next(mId, !bought);
        }));

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

    // Upgrade unlock constraints
    mParamSubs.push_back(states[State::BoughtSecondT1].subscribe(
        [this](bool bought) { mTimeWarpUp->setActive(bought); }));
}

void PowerWizard::onRender(SDL_Renderer* r) {
    TextureBuilder tex;

    if (mFreezeFireball) {
        mFreezeFireball->draw(tex);
    }

    WizardBase::onRender(r);
}

void PowerWizard::onHide(bool hide) {
    WizardBase::onHide(hide);
    mFireballs->clear();
}

void PowerWizard::onTargetHide(WizardId id, bool hide) {
    if (hide) {
        mFireballs->remove(id);
    }
}

void PowerWizard::onT1Reset() {
    mFireballs->clear();

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
            TimeWizard::Params::get(TimeWizard::Param::FreezeEffect)
                .get();
        mFreezeFireball->applyTimeEffect(freezeEffect);
        mFireballs->push_back(std::move(mFreezeFireball));
    }
}

void PowerWizard::shootFireball() {
    WizardId target = getTarget();
    auto data = newFireballData(target);

    if (!ParameterSystem::Param(State::TimeWizFrozen).get()) {
        mFireballs->push_back(std::move(ComponentFactory<Fireball>::New(
            SDL_FPoint{mPos->rect.cX(), mPos->rect.cY()}, target, data)));
    } else {
        if (!mFreezeFireball) {
            mFreezeFireball = std::move(ComponentFactory<Fireball>::New(
                SDL_FPoint{mPos->rect.cX(), mPos->rect.cY()}, target, data));
        } else {
            mFreezeFireball->addFireball(data);
        }
    }
}

void PowerWizard::shootFireball(SDL_FPoint target) {
    size_t size = mFireballs->size();
    shootFireball();
    if (size < mFireballs->size()) {
        mFireballs->back().launch(target);
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

FireballData PowerWizard::newFireballData(WizardId target) {
    PowerWizard::Params params;
    Number speedEffect = params[PowerWizard::Param::FBSpeedEffect].get();

    FireballData data;
    switch (target) {
        case WIZARD:
            data.duration = params[PowerWizard::Param::Duration].get();
            data.power = params[PowerWizard::Param::Power].get() * speedEffect;
            break;
        case TIME_WIZARD:
            data.power =
                (params[PowerWizard::Param::Power].get() * speedEffect) ^
                params[PowerWizard::Param::TimeWarpUp].get();
            break;
        case CRYSTAL:
            data.duration = params[PowerWizard::Param::Duration].get() * 2;
            data.power =
                params[PowerWizard::Param::FireRingEffect].get() * speedEffect;
            break;
    };
    data.sizeFactor = 1;
    data.speed = params[PowerWizard::Param::FBSpeed].get().toFloat();
    return data;
}

Number PowerWizard::calcPower() {
    PowerWizard::Params params;
    return params[PowerWizard::Param::BasePower].get() *
           params[PowerWizard::Param::PowerUp].get() *
           max(1, params[PowerWizard::Param::Speed].get() * 16 / 1000);
}

Number PowerWizard::calcSpeed() {
    PowerWizard::Params params;
    TimeWizard::Params timeParams;
    Number timeEffect = timeParams[TimeWizard::Param::SpeedEffect].get();
    return params[PowerWizard::Param::BaseSpeed].get() * timeEffect;
}

Number PowerWizard::calcFBSpeed() {
    PowerWizard::Params params;
    TimeWizard::Params timeParams;

    return params[PowerWizard::Param::BaseFBSpeed].get() *
           timeParams[TimeWizard::Param::FBSpeedUp].get();
}

Number PowerWizard::calcFBSpeedEffect() {
    PowerWizard::Params params;
    Number fbSpeed = max(params[PowerWizard::Param::FBSpeed].get(), 1);

    Number twoFbSpeed = 2 * fbSpeed;

    return fbSpeed * (twoFbSpeed ^ (twoFbSpeed - 2));
}

void PowerWizard::calcTimer() {
    Timer& timer = mFireballTimerSub->get<TimerObservable::DATA>();
    float div = PowerWizard::Params::get(PowerWizard::Param::Speed)
                    .get()
                    .toFloat();
    if (div <= 0) {
        div = 1;
    }
    timer.length = fmax(1000 / div, 16);
}

Number PowerWizard::calcFireRingEffect() {
    PowerWizard::Params params;
    return (params[PowerWizard::Param::Power].get() / 5) + 1;
}

void PowerWizard::setPos(float x, float y) {
    WizardBase::setPos(x, y);
    if (mFreezeFireball) {
        mFreezeFireball->setPos(mPos->rect.cX(), mPos->rect.cY());
    }
}
}  // namespace PowerWizard
