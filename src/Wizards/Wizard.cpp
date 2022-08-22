#include "Wizard.h"

// Wizard
const std::string Wizard::POWER_UP_IMG = "res/upgrades/fireball_upgrade.png";
const std::string Wizard::MULTI_UP_IMG = "res/upgrades/multi_upgrade.png";
const std::string Wizard::CRIT_UP_IMG = "res/upgrades/crit_upgrade.png";
const std::string Wizard::POWER_BKGRND = "res/wizards/power_effect_bkgrnd.png";
const std::string Wizard::FIREBALL_IMG = "res/projectiles/fireball.png";
const std::string Wizard::FIREBALL_BUFFED_IMG =
    "res/projectiles/fireball_buffed.png";

Wizard::Wizard() : WizardBase(WIZARD) {}

void Wizard::init() {
    mPowBkgrnd.texture = AssetManager::getTexture(POWER_BKGRND);

    WizardBase::init();
}
void Wizard::setDefaultValues() {
    ParameterSystem::Params<WIZARD> params;
    params.set(WizardParams::BasePower, 1);
    params.set(WizardParams::BaseSpeed, 1);
    params.set(WizardParams::PowerWizEffect, 1);
    params.set(WizardParams::BaseCrit, 1);
    params.set(WizardParams::BaseCritSpread, 0);
}
void Wizard::setSubscriptions() {
    mFireballTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            std::bind(&Wizard::onTimer, this, std::placeholders::_1),
            Timer(1000));
    mFireballSub =
        ServiceSystem::Get<FireballService, Fireball::HitObservable>()
            ->subscribe(
                std::bind(&Wizard::onFireballHit, this, std::placeholders::_1),
                mId);
    mFireballFireRingSub =
        ServiceSystem::Get<FireballService, Fireball::FireRingHitObservable>()
            ->subscribe(std::bind(&Wizard::onFireballFireRingHit, this,
                                  std::placeholders::_1, std::placeholders::_2),
                        mId);
    mFreezeSub = TimeSystem::GetFreezeObservable()->subscribe(
        std::bind(&Wizard::onFreeze, this, std::placeholders::_1),
        std::bind(&Wizard::onUnfreeze, this, std::placeholders::_1));
    attachSubToVisibility(mFireballTimerSub);
    attachSubToVisibility(mFireballSub);
    attachSubToVisibility(mFireballFireRingSub);
    attachSubToVisibility(mFreezeSub);
}
void Wizard::setUpgrades() {
    // Power Display
    UpgradePtr up = std::make_shared<Upgrade>();
    up->setMaxLevel(0)
        .setEffectSource(params[WizardParams::Power],
                         Upgrade::Defaults::MultiplicativeEffect)
        .setImage(WIZ_IMGS.at(mId))
        .setDescription("Power");
    mPowerDisplay = mUpgrades->subscribe(up);

    // Target Upgrade
    up = std::make_shared<Upgrade>();
    up->setMaxLevel(-1)
        .setImage(WIZ_IMGS.at(mTarget))
        .setDescription("Change the Wizard's target");
    mTargetUp = mUpgrades->subscribe(
        [this](UpgradePtr u) {
            if (u->getLevel() % 2 == 0 || WizardSystem::Hidden(CATALYST)) {
                mTarget = CRYSTAL;
                u->setLevel(0);
            } else {
                mTarget = CATALYST;
                u->setLevel(1);
            }
            u->setEffect("Target: " + WIZ_NAMES.at(mTarget))
                .setImage(WIZ_IMGS.at(mTarget));
        },
        up);

    // Power Upgrade
    up = std::make_shared<Upgrade>();
    up->setMaxLevel(5)
        .setCostSource(
            params[WizardParams::PowerUpCost)]
        .setMoneySource(Upgrade::Defaults::CRYSTAL_MAGIC)
        .setEffectSource(params[WizardParams::PowerUp],
                         Upgrade::Defaults::AdditiveEffect)
        .setImage(POWER_UP_IMG)
        .setDescription("Increase Wizard base power by 1");
    mPowerUp = mUpgrades->subscribe(
        [](UpgradePtr u) {
            params[WizardParams::PowerUp].set(u->getLevel());
            u->getCostSource()->set(25 * (Number(1.75) ^ u->getLevel()));
        },
        up);

    // Crit Upgrade
    up = std::make_shared<Upgrade>();
    up->setMaxLevel(10)
        .setCostSource(params[WizardParams::CritUpCost)]
        .setMoneySource(Upgrade::Defaults::CRYSTAL_MAGIC)
        .setEffectSource(
            ParameterSystem::ParamMap<WIZARD>(
                {WizardParams::CritUp, WizardParams::CritSpreadUp}),
            []() {
        ParameterSystem::Params<WIZARD> params;
        std::stringstream ss;
        ss << "Crit: +" << params[WizardParams::CritUp].get() << " | Spread: *"
           << params[WizardParams::CritSpreadUp].get();
        return ss.str();
            })
        .setImage(CRIT_UP_IMG)
        .setDescription(
            "Multiplies critical hit amount *1.1 and increases chance for "
            "higher crits");
    mCritUp = mUpgrades->subscribe(
        [this](UpgradePtr u) {
            ParameterSystem::Params<WIZARD> params;
            params.set(WizardParams::CritUp, (Number(1.1) ^ u->getLevel()) - 1);
            params.set(WizardParams::CritSpreadUp, Number(.95) ^ u->getLevel());
            u->getCostSource()->set(100 * (Number(1.5) ^ u->getLevel()));
        },
        up);

    // Multi Upgrade
    up = std::make_shared<Upgrade>();
    up->setMaxLevel(20)
        .setCostSource(
            params[WizardParams::MultiUpCost)]
        .setMoneySource(Upgrade::Defaults::CRYSTAL_MAGIC)
        .setEffectSource(params[WizardParams::MultiUp],
                         Upgrade::Defaults::PercentEffect)
        .setImage(MULTI_UP_IMG)
        .setDescription("Increase double fireball chance by +5%");
    mMultiUp = mUpgrades->subscribe(
        [](UpgradePtr u) {
            params[WizardParams::MultiUp].set(Number(.05) * u->getLevel());
            u->getCostSource()->set(150 * (Number(1.4) ^ u->getLevel()));
        },
        up);
}
void Wizard::setParamTriggers() {
    mParamSubs.push_back(
        ParameterSystem::ParamMap<WIZARD, CRYSTAL, CATALYST>(
            {WizardParams::BasePower, WizardParams::PowerUp,
             WizardParams::Speed, WizardParams::PowerWizEffect},
            {CrystalParams::MagicEffect}, {CatalystParams::MagicEffect})
            .subscribe([this]() { calcPower(); }));
    mParamSubs.push_back(
        ParameterSystem::ParamMap<WIZARD, TIME_WIZARD>(
            {WizardParams::BaseSpeed}, {TimeWizardParams::SpeedEffect})
            .subscribe([this]() { calcSpeed(); }));
    mParamSubs.push_back(
        params[WizardParams::Speed).subscribe([this](] {
        calcTimer();
        }));
    mParamSubs.push_back(ParameterSystem::ParamMap<WIZARD>(
                             {WizardParams::BaseCrit, WizardParams::CritUp})
                             .subscribe([this]() { calcCrit(); }));
    mParamSubs.push_back(
        ParameterSystem::ParamMap<WIZARD>(
            {WizardParams::BaseCritSpread, WizardParams::CritSpreadUp})
            .subscribe([this]() { calcCritSpread(); }));
}
void Wizard::setEventTriggers() {
    WizardSystem::States states;
    mStateSubs.push_back(
        states.subscribe(WizardSystem::State::BoughtFirstT1, [this](bool val) {
            UpgradeList::Get(mPowerUp)->setMaxLevel(val ? 10 : 5).updateInfo();
        }));
    mStateSubs.push_back(
        states.subscribe(WizardSystem::State::BoughtPowerWizard,
                         [this](bool val) { mCritUp->setActive(val); }));
    mStateSubs.push_back(
        states.subscribe(WizardSystem::State::BoughtTimeWizard,
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
    WizardBase::onResetT1();

    mFireballs.clear();
    mFireballFreezeCnt = 0;
    mPowWizBoosts.clear();

    if (mFireballTimerSub) {
        mFireballTimerSub->get<TimerObservable::DATA>().reset();
    }
    mPowWizTimerSub.reset();
}

bool Wizard::onTimer(Timer& timer) {
    shootFireball();
    float Multi =
        params[WizardParams::MultiUp).get().toFloat(];
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

            params[WizardParams::PowerWizEffect].set(
                mPowWizBoosts.front().first);
            mPowWizTimerSub = TimeSystem::GetTimerObservable()->subscribe(
                std::bind(&Wizard::onPowWizTimer, this, std::placeholders::_1),
                std::bind(&Wizard::onPowWizTimerUpdate, this,
                          std::placeholders::_1, std::placeholders::_2),
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
    ParameterSystem::Param<WIZARD> powerWizEffect(WizardParams::PowerWizEffect);
    mPowWizBoosts.erase(mPowWizBoosts.begin());
    if (mPowWizBoosts.empty()) {
        powerWizEffect.set(1);
        return false;
    }

    powerWizEffect.set(mPowWizBoosts.begin()->first);
    timer.length = mPowWizBoosts.begin()->second.toFloat();
    return true;
}
void Wizard::onPowWizTimerUpdate(Time dt, Timer timer) {
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

void Wizard::calcPower() {
    ParameterSystem::Params<WIZARD> wizParams;
    Number crystalEff =
        params[CrystalParams::MagicEffect).get(];
    Number catalystEff =
        params[CatalystParams::MagicEffect).get(];
    Number power = (wizParams.get(WizardParams::BasePower) +
                    wizParams.get(WizardParams::PowerUp)) *
                   crystalEff * catalystEff *
                   wizParams.get(WizardParams::PowerWizEffect) *
                   max(1, wizParams.get(WizardParams::Speed) * 16 / 1000);
    wizParams.set(WizardParams::Power, power);
}

void Wizard::calcSpeed() {
    ParameterSystem::Params<WIZARD> wizParams;
    Number timeEffect = params[TimeWizardParams::SpeedEffect].get();
    Number speed = wizParams.get(WizardParams::BaseSpeed) * timeEffect;
    wizParams.set(WizardParams::Speed, speed);
}

void Wizard::calcTimer() {
    Timer& timer = mFireballTimerSub->get<TimerObservable::DATA>();
    float div =
        params[WizardParams::Speed).get().toFloat(];
    if (div <= 0) {
        div = 1;
    }
    timer.length = fmax(1000 / div, 16);
}

void Wizard::calcCrit() {
    ParameterSystem::Params<WIZARD> params;
    Number crit =
        params[WizardParams::BaseCrit) + params.get(WizardParams::CritUp].get();
    params.set(WizardParams::Crit, crit);
}

void Wizard::calcCritSpread() {
    ParameterSystem::Params<WIZARD> params;
    Number critSpread = params[WizardParams::BaseCritSpread].get() +
                        params[WizardParams::CritSpreadUp].get();
    params.set(WizardParams::CritSpread, critSpread);
}

Wizard::FireballData Wizard::newFireball() {
    ParameterSystem::Params<WIZARD> params;
    Number power = params[WizardParams::Power].get();

    if (UpgradeList::Get(mCritUp)->getLevel() == 0) {
        return {power, 1.0};
    }

    float frac = rDist(gen);
    if (frac == 0) {
        frac = std::numeric_limits<float>::min();
    }
    frac = (frac ^ params[WizardParams::CritSpread)).toFloat(].get() + .5;
    return {power * params[WizardParams::Crit) * frac, powf(frac, .25].get()};
}

void Wizard::setPos(float x, float y) {
    WizardBase::setPos(x, y);
    if (TimeSystem::Frozen(TimeSystem::FreezeType::TIME_WIZARD) &&
        mFireballFreezeCnt > 0) {
        mFireballs.back()->setPos(mPos->rect.cX(), mPos->rect.cY());
    }
}
