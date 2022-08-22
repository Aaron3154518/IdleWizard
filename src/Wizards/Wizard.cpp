#include "Wizard.h"

// Wizard
const std::string Wizard::POWER_UP_IMG = "res/upgrades/fireball_upgrade.png";
const std::string Wizard::MULTI_UP_IMG = "res/upgrades/multi_upgrade.png";
const std::string Wizard::CRIT_UP_IMG = "res/upgrades/crit_upgrade.png";
const std::string Wizard::POWER_BKGRND = "res/wizards/power_effect_bkgrnd.png";
const std::string Wizard::FIREBALL_IMG = "res/projectiles/fireball.png";
const std::string Wizard::FIREBALL_BUFFED_IMG =
    "res/projectiles/fireball_buffed.png";

const std::vector<WizardId> Wizard::TARGETS = {CRYSTAL, CATALYST};

void Wizard::setDefaults() {
    ParameterSystem::Params<WIZARD> params;

    // Default 0
    params[WizardParams::BaseCritSpread]->setDefault(0);

    // Default 1
    params.setDefaults({WizardParams::BasePower, WizardParams::BasePower,
                        WizardParams::BaseCrit, WizardParams::PowerWizEffect},
                       1);
}

Wizard::Wizard() : WizardBase(WIZARD) {}

void Wizard::init() {
    mPowBkgrnd.texture = AssetManager::getTexture(POWER_BKGRND);

    WizardBase::init();
}
void Wizard::setSubscriptions() {
    mFireballTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            [this](Timer& t) { return onTimer(t); }, Timer(1000));
    mFireballSub =
        ServiceSystem::Get<FireballService, Fireball::HitObservable>()
            ->subscribe([this](const Fireball& f) { onFireballHit(f); }, mId);
    mFireballFireRingSub =
        ServiceSystem::Get<FireballService, Fireball::FireRingHitObservable>()
            ->subscribe(
                [this](Fireball& f, const Number& e) {
                    onFireballFireRingHit(f, e);
                },
                mId);
    mFreezeSub = TimeSystem::GetFreezeObservable()->subscribe(
        [this](TimeSystem::FreezeType f) { onFreeze(f); },
        [this](TimeSystem::FreezeType f) { onUnfreeze(f); });
    attachSubToVisibility(mFireballTimerSub);
    attachSubToVisibility(mFireballSub);
    attachSubToVisibility(mFireballFireRingSub);
    attachSubToVisibility(mFreezeSub);
}
void Wizard::setUpgrades() {
    ParameterSystem::Params<WIZARD> params;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(WIZ_IMGS.at(mId));
    dUp->setDescription("Power");
    dUp->setEffect(params[WizardParams::Power],
                   Upgrade::Defaults::MultiplicativeEffect);
    mPowerDisplay = mUpgrades->subscribe(dUp);

    // Target Upgrade
    TogglePtr tUp = std::make_shared<Toggle>(
        [this](unsigned int lvl, Toggle& tUp) {
            mTarget = TARGETS.at(lvl);
            if (mTarget == CRYSTAL || !WizardSystem::Hidden(mTarget)) {
                tUp.setImage(WIZ_IMGS.at(mTarget));
                tUp.setInfo("Target: " + WIZ_NAMES.at(mTarget));
            } else {
                tUp.setLevel(lvl + 1);
            }
        },
        TARGETS.size());
    tUp->setDescription("Change the Wizard's target");
    mTargetUp = mUpgrades->subscribe(tUp);

    // Power Upgrade
    UpgradePtr up = std::make_shared<Upgrade>(
        params[WizardParams::PowerUpLvl], params[WizardParams::PowerUpMaxLvl]);
    up->setImage(POWER_UP_IMG);
    up->setDescription("Increase Wizard base power by 1");
    up->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                params[WizardParams::PowerUpCost],
                [](const Number& lvl) { return 25 * (1.75 ^ lvl); });
    up->setEffect(
        params[WizardParams::PowerUp], [](const Number& lvl) { return lvl; },
        Upgrade::Defaults::AdditiveEffect);
    mPowerUp = mUpgrades->subscribe(up);

    // Crit Upgrade
    up = std::make_shared<Upgrade>(params[WizardParams::CritUpLvl], 10);
    up->setImage(CRIT_UP_IMG);
    up->setDescription(
        "Multiplies critical hit amount *1.1 and increases chance for "
        "higher crits");
    up->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                params[WizardParams::CritUpCost],
                [](const Number& lvl) { return 100 * (1.5 ^ lvl); });
    up->setEffects({{params[WizardParams::CritUp],
                     [](const Number& lvl) { return (1.1 ^ lvl) - 1; }},
                    {params[WizardParams::CritSpreadUp],
                     [](const Number& lvl) { return .95 ^ lvl; }}},
                   {}, []() {
                       ParameterSystem::Params<WIZARD> params;
                       std::stringstream ss;
                       ss << "Crit: +" << params[WizardParams::CritUp].get()
                          << " | Spread: *"
                          << params[WizardParams::CritSpreadUp].get();
                       return ss.str();
                   });
    mCritUp = mUpgrades->subscribe(up);

    // Multi Upgrade
    up = std::make_shared<Upgrade>(params[WizardParams::MultiUpLvl], 20);
    up->setImage(MULTI_UP_IMG);
    up->setDescription("Increase double fireball chance by +5%");
    up->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                params[WizardParams::MultiUpCost],
                [](const Number& lvl) { return 150 * (1.4 ^ lvl); });
    up->setEffect(
        params[WizardParams::MultiUp],
        [](const Number& lvl) { return .05 * lvl; },
        Upgrade::Defaults::PercentEffect);
    mMultiUp = mUpgrades->subscribe(up);
}
void Wizard::setParamTriggers() {
    ParameterSystem::Params<WIZARD> params;
    ParameterSystem::Params<CRYSTAL> crystalParams;
    ParameterSystem::Params<CATALYST> catalystParams;
    ParameterSystem::Params<TIME_WIZARD> timeParams;
    ParameterSystem::States states;

    mParamSubs.push_back(params[WizardParams::Power].subscribeTo(
        {params[WizardParams::BasePower], params[WizardParams::PowerUp],
         params[WizardParams::Speed], params[WizardParams::PowerWizEffect],
         crystalParams[CrystalParams::MagicEffect],
         catalystParams[CatalystParams::MagicEffect]},
        {}, [this]() { return calcPower(); }));

    mParamSubs.push_back(params[WizardParams::Speed].subscribeTo(
        {params[WizardParams::BaseSpeed],
         timeParams[TimeWizardParams::SpeedEffect]},
        {}, [this]() { return calcSpeed(); }));

    mParamSubs.push_back(
        params[WizardParams::Speed].subscribe([this]() { calcTimer(); }));

    mParamSubs.push_back(params[WizardParams::Crit].subscribeTo(
        {params[WizardParams::BaseCrit, WizardParams::CritUp]}, {},
        [this]() { return calcCrit(); }));

    mParamSubs.push_back(params[WizardParams::CritSpread].subscribeTo(
        {params[WizardParams::BaseCritSpread, WizardParams::CritSpreadUp]}, {},
        [this]() { return calcCritSpread(); }));

    mParamSubs.push_back(params[WizardParams::PowerUpMaxLvl].subscribeTo(
        states[State::BoughtFirstT1],
        [this](bool val) { return val ? 10 : 5; }));

    mParamSubs.push_back(states[State::BoughtPowerWizard].subscribe(
        [this](bool val) { mCritUp->setActive(val); }));

    mParamSubs.push_back(states[State::BoughtTimeWizard].subscribe(
        [this](bool val) { mMultiUp->setActive(val); }));
}

void Wizard::onRender(SDL_Renderer* r) {
    if (mPowWizTimerSub &&
        mPowWizTimerSub->get<TimerObservable::DATA>().isActive()) {
        mPowBkgrnd.dest = mPos->rect;
        mPowBkgrnd.shrinkToTexture();
        TextureBuilder().draw(mPowBkgrnd);
    }

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

void Wizard::onHide(WizardId id, bool hide) {
    WizardBase::onHide(id, hide);
    if (hide) {
        switch (id) {
            case WIZARD:
                mFireballs.clear();
                break;
            default:
                std::remove_if(mFireballs.begin(), mFireballs.end(),
                               [id](const FireballPtr& ball) {
                                   return ball->getTargetId() == id;
                               });
                break;
        }
        if (id == mTarget) {
            mTarget = CRYSTAL;
        }
    }
}

void Wizard::onResetT1() {
    mFireballs.clear();
    mFireballFreezeCnt = 0;
    mPowWizBoosts.clear();

    if (mFireballTimerSub) {
        mFireballTimerSub->get<TimerObservable::DATA>().reset();
    }
    mPowWizTimerSub.reset();
}

bool Wizard::onTimer(Timer& timer) {
    ParameterSystem::Params<WIZARD> params;

    shootFireball();
    float Multi = params[WizardParams::MultiUp].get().toFloat();
    if (rDist(gen) < Multi) {
        shootFireball(
            SDL_FPoint{(rDist(gen) - .5f) * mPos->rect.w() + mPos->rect.cX(),
                       (rDist(gen) - .5f) * mPos->rect.h() + mPos->rect.cY()});
    }
    return true;
}

void Wizard::onFireballHit(const Fireball& fireball) {
    switch (fireball.getSourceId()) {
        case POWER_WIZARD: {
            Number power = fireball.getValue(PowerWizardParams::Power),
                   duration = fireball.getValue(PowerWizardParams::Duration);
            for (auto it = mPowWizBoosts.begin(), end = mPowWizBoosts.end();
                 duration > 0 && it != end; ++it) {
                if (power >= it->first) {  // Replace current
                    if (power == it->first &&
                        duration <= it->first) {  // No change
                        break;
                    }
                    if (duration < it->second) {  // Reduce current
                        mPowWizBoosts.insert(
                            it + 1,
                            std::make_pair(it->first, it->second - duration));
                    } else {  // Remove current and
                              // update future
                        Number durationLeft = duration - it->second;
                        for (auto it2 = it + 1; durationLeft > 0 && it2 != end;
                             ++it2) {
                            if (durationLeft < it2->second) {  // Reduce future
                                it2->second -= durationLeft;
                                durationLeft = 0;
                            } else {  // Remove future
                                durationLeft -= it2->second;
                                it2 = mPowWizBoosts.erase(it2);
                                if (it2 == end) {
                                    break;
                                }
                            }
                        }
                    }
                    it->first = power;
                    it->second = duration;
                }
                duration -= it->second;
            }
            if (duration > 0) {
                mPowWizBoosts.push_back(std::make_pair(power, duration));
            }

            ParameterSystem::Params<WIZARD> params;
            params[WizardParams::PowerWizEffect].set(
                mPowWizBoosts.front().first);
            mPowWizTimerSub = TimeSystem::GetTimerObservable()->subscribe(
                [this](Timer& t) { return onPowWizTimer(t); },
                [this](Time dt, Timer& t) { onPowWizTimerUpdate(dt, t); },
                Timer(mPowWizBoosts.front().second.toFloat()));
        } break;
    }
}

void Wizard::onFireballFireRingHit(Fireball& fireball,
                                   const Number& fireRingEffect) {
    fireball.getValue() *= fireRingEffect;
    fireball.setSize(fireball.getSize() * 1.15);
}

bool Wizard::onPowWizTimer(Timer& timer) {
    auto powerWizEffect =
        ParameterSystem::Param<WIZARD>(WizardParams::PowerWizEffect);
    mPowWizBoosts.erase(mPowWizBoosts.begin());
    if (mPowWizBoosts.empty()) {
        powerWizEffect.set(1);
        return false;
    }

    powerWizEffect.set(mPowWizBoosts.begin()->first);
    timer.length = mPowWizBoosts.begin()->second.toFloat();
    return true;
}
void Wizard::onPowWizTimerUpdate(Time dt, Timer& timer) {
    mPowWizBoosts.begin()->second = timer.getTimeLeft();
}

void Wizard::onFreeze(TimeSystem::FreezeType type) {
    switch (type) {
        case TimeSystem::FreezeType::TIME_WIZARD:
            mFireballFreezeCnt = 0;
            break;
    };
}
void Wizard::onUnfreeze(TimeSystem::FreezeType type) {
    switch (type) {
        case TimeSystem::FreezeType::TIME_WIZARD:
            if (mFireballFreezeCnt > 0 && !mFireballs.empty()) {
                Number freezeEffect = ParameterSystem::Param<TIME_WIZARD>(
                                          TimeWizardParams::FreezeEffect)
                                          .get();
                FireballPtr& fireball = mFireballs.back();
                fireball->getValue() ^= freezeEffect;
            }
            break;
    }
}

void Wizard::shootFireball() {
    bool frozen = TimeSystem::Frozen(TimeSystem::FreezeType::TIME_WIZARD);
    if (!frozen || mFireballFreezeCnt == 0) {
        FireballData fp = newFireball();
        mFireballs.push_back(std::move(ComponentFactory<Fireball>::New(
            SDL_FPoint{mPos->rect.cX(), mPos->rect.cY()}, mId, mTarget,
            mPowWizBoosts.empty() ? FIREBALL_IMG : FIREBALL_BUFFED_IMG,
            fp.power)));
        auto& fireball = mFireballs.back();
        fireball->setSize(fireball->getSize() * fp.sizeFactor);
        if (!mPowWizBoosts.empty()) {
            fireball->getState(Fireball::State::PowerWizBoosted) = true;
        }
    }
    if (frozen) {
        auto& fireball = mFireballs.back();
        if (++mFireballFreezeCnt > 1) {
            fireball->getValue() += newFireball().power;
        }
        fireball->setSize(fmin(pow(mFireballFreezeCnt, 1.0 / 3.0), 10));
    }
}

void Wizard::shootFireball(SDL_FPoint target) {
    size_t size = mFireballs.size();
    shootFireball();
    if (size != mFireballs.size()) {
        mFireballs.back()->launch(target);
    }
}

Number Wizard::calcPower() {
    ParameterSystem::Params<WIZARD> params;
    ParameterSystem::Params<CRYSTAL> crystalParams;
    ParameterSystem::Params<CATALYST> catalystParams;

    return (params[WizardParams::BasePower].get() +
            params[WizardParams::PowerUp].get()) *
           crystalParams[CrystalParams::MagicEffect].get() *
           catalystParams[CatalystParams::MagicEffect].get() *
           params[WizardParams::PowerWizEffect].get() *
           max(1, params[WizardParams::Speed].get() * 16 / 1000);
}

Number Wizard::calcSpeed() {
    ParameterSystem::Params<WIZARD> params;
    ParameterSystem::Params<TIME_WIZARD> timeParams;

    return params[WizardParams::BaseSpeed].get() *
           timeParams[TimeWizardParams::SpeedEffect].get();
}

void Wizard::calcTimer() {
    ParameterSystem::Params<WIZARD> params;

    Timer& timer = mFireballTimerSub->get<TimerObservable::DATA>();
    float div = params[WizardParams::Speed].get().toFloat();
    if (div <= 0) {
        div = 1;
    }
    timer.length = fmax(1000 / div, 16);
}

Number Wizard::calcCrit() {
    ParameterSystem::Params<WIZARD> params;
    return params[WizardParams::BaseCrit].get() +
           params[WizardParams::CritUp].get();
}

Number Wizard::calcCritSpread() {
    ParameterSystem::Params<WIZARD> params;
    return params[WizardParams::BaseCritSpread].get() +
           params[WizardParams::CritSpreadUp].get();
}

Wizard::FireballData Wizard::newFireball() {
    ParameterSystem::Params<WIZARD> params;
    Number power = params[WizardParams::Power].get();

    if (params[WizardParams::CritUpLvl].get() == 0) {
        return {power, 1.0};
    }

    float frac = rDist(gen);
    if (frac == 0) {
        frac = std::numeric_limits<float>::min();
    }
    frac = (frac ^ params[WizardParams::CritSpread].get()).toFloat() + .5;
    return {power * (params[WizardParams::Crit].get() * frac), powf(frac, .25)};
}

void Wizard::setPos(float x, float y) {
    WizardBase::setPos(x, y);
    if (TimeSystem::Frozen(TimeSystem::FreezeType::TIME_WIZARD) &&
        mFireballFreezeCnt > 0) {
        mFireballs.back()->setPos(mPos->rect.cX(), mPos->rect.cY());
    }
}
