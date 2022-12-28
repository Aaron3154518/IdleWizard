#include "Wizard.h"

namespace Wizard {
// Wizard
Wizard::Wizard() : WizardBase(WIZARD) {}
void Wizard::init() {
    mFireballs = ComponentFactory<FireballList>::New();

    mImg.set(Constants::IMG());
    mImg.setDest(IMG_RECT);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);

    mPowBkgrnd.set(Constants::POWER_BKGRND());

    WizardBase::init();
}
void Wizard::setSubscriptions() {
    mFireballTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            [this](Timer& t) { return onTimer(t); }, Timer(1000));
    mPowFireballHitSub =
        PowerWizard::FireballList::GetHitObservable()->subscribe(
            [this](const PowerWizard::Fireball& fb) { onPowFireballHit(fb); },
            [this](const PowerWizard::Fireball& fb) {
                return PowerWizard::Fireball::filter(fb, mId);
            },
            mPos);
    mAnimTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            mImg->nextFrame();
            WizardSystem::GetWizardImageObservable()->next(mId, mImg);
            return true;
        },
        Constants::IMG());
    mPowBkAnimTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            mPowBkgrnd->nextFrame();
            return true;
        },
        Constants::POWER_BKGRND());
    mT1ResetSub = WizardSystem::GetWizardEventObservable()->subscribe(
        [this]() { onT1Reset(); }, WizardSystem::Event::ResetT1);
    mTimeWarpSub = WizardSystem::GetWizardEventObservable()->subscribe(
        [this]() { onTimeWarp(); }, WizardSystem::Event::TimeWarp);
    mTargetHideSub = WizardSystem::GetHideObservable()->subscribeToAll(
        [this](WizardId id, bool hide) { onTargetHide(id, hide); });
    mGlobHitSub = PoisonWizard::Glob::GetHitObservable()->subscribe(
        [this]() { onGlobHit(); }, mPos);
    attachSubToVisibility(mFireballTimerSub);
    attachSubToVisibility(mPowFireballHitSub);
    attachSubToVisibility(mTimeWarpSub);
    attachSubToVisibility(mGlobHitSub);
}
void Wizard::setUpgrades() {
    ParameterSystem::Params<WIZARD> params;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(mId);
    dUp->setEffects(
        {params[WizardParams::Power], params[WizardParams::FBSpeed],
         params[WizardParams::FBSpeedEffect], params[WizardParams::Speed],
         params[WizardParams::Crit]},
        {}, []() -> TextUpdateData {
            ParameterSystem::Params<WIZARD> params;
            std::stringstream ss;
            std::vector<RenderTextureCPtr> imgs;
            ss << "Power: "
               << UpgradeDefaults::MultiplicativeEffectText(
                      params[WizardParams::Power].get())
               << "\nFire Rate: " << params[WizardParams::Speed].get()
               << "/s\n{i} Speed: "
               << UpgradeDefaults::MultiplicativeEffectText(
                      params[WizardParams::FBSpeed].get())
               << ", {b}Power : "
               << UpgradeDefaults::MultiplicativeEffectText(
                      params[WizardParams::FBSpeedEffect].get())
               << "\nCrit Power: " << params[WizardParams::Crit].get();
            imgs.push_back(IconSystem::Get(Constants::FB_IMG()));
            return {ss.str(), imgs};
        });
    mPowerDisplay = mUpgrades->subscribe(dUp);

    // Target Upgrade
    TogglePtr tUp = std::make_shared<Toggle>(
        [this](unsigned int lvl, Toggle& tUp) {
            mTarget = Constants::TARGETS.at(lvl);
            if (mTarget == CRYSTAL || !WizardSystem::Hidden(mTarget)) {
                tUp.setImage(WIZ_IMGS.at(mTarget));
                tUp.setEffect({"Target: " + WIZ_NAMES.at(mTarget)});
            } else {
                tUp.setLevel(lvl + 1);
            }
        },
        Constants::TARGETS.size());
    tUp->setDescription({"Change the target"});
    mTargetUp = mUpgrades->subscribe(tUp);

    // Power Upgrade
    UpgradePtr up = std::make_shared<Upgrade>(
        params[WizardParams::PowerUpLvl], params[WizardParams::PowerUpMaxLvl]);
    up->setImage(Constants::POWER_UP_IMG);
    up->setDescription({"Increase base power by 1"});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                params[WizardParams::PowerUpCost]);
    up->setEffects(params[WizardParams::PowerUp],
                   UpgradeDefaults::AdditiveEffect);
    mParamSubs.push_back(params[WizardParams::PowerUpCost].subscribeTo(
        up->level(), [](const Number& lvl) { return 25 * (1.55 ^ lvl); }));
    mParamSubs.push_back(params[WizardParams::PowerUp].subscribeTo(
        up->level(),
        [](const Number& lvl) { return min(lvl, 5) + 2 * max(lvl - 5, 0); }));
    mPowerUp = mUpgrades->subscribe(up);

    // Crit Upgrade
    up = std::make_shared<Upgrade>(params[WizardParams::CritUpLvl], 10);
    up->setImage(Constants::CRIT_UP_IMG);
    up->setDescription(
        {"Multiplies critical hit power *1.1 and increases chance for "
         "higher crits"});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                params[WizardParams::CritUpCost]);
    up->setEffects(
        {params[WizardParams::CritUp], params[WizardParams::CritSpreadUp]}, {},
        []() -> TextUpdateData {
            ParameterSystem::Params<WIZARD> params;
            std::stringstream ss;
            ss << "Crit: +" << params[WizardParams::CritUp].get()
               << " | Spread: *" << params[WizardParams::CritSpreadUp].get();
            return {ss.str()};
        });
    mParamSubs.push_back(UpgradeDefaults::subscribeT1UpCost(
        up->level(), params[WizardParams::CritUpCost],
        [](const Number& lvl) { return 200 * (1.5 ^ lvl); }));
    mParamSubs.push_back(params[WizardParams::CritUp].subscribeTo(
        up->level(), [](const Number& lvl) { return (1.1 ^ lvl) - 1; }));
    mParamSubs.push_back(params[WizardParams::CritSpreadUp].subscribeTo(
        up->level(), [](const Number& lvl) { return .95 ^ lvl; }));
    mCritUp = mUpgrades->subscribe(up);

    // Robo Crit Upgrade
    up = std::make_shared<Upgrade>(params[WizardParams::RoboCritUpLvl], 10);
    up->setImage("");
    up->setDescription({"Multiplies critical hit power *1.15"});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                params[WizardParams::RoboCritUpCost]);
    up->setEffects(params[WizardParams::RoboCritUp],
                   UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(params[WizardParams::RoboCritUpCost].subscribeTo(
        up->level(),
        [](const Number& lvl) { return Number(1, 20) * (11 ^ lvl); }));
    mParamSubs.push_back(params[WizardParams::RoboCritUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1.15 ^ lvl; }));
    mRoboCritUp = mUpgrades->subscribe(up);

    // Multi Upgrade
    up = std::make_shared<Upgrade>(params[WizardParams::MultiUpLvl], 20);
    up->setImage(Constants::MULTI_UP_IMG);
    up->setDescription({"Increase double {i} chance by +5%",
                        {IconSystem::Get(Constants::FB_IMG())}});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                params[WizardParams::MultiUpCost]);
    up->setEffects(params[WizardParams::MultiUp],
                   UpgradeDefaults::PercentEffect);
    mParamSubs.push_back(UpgradeDefaults::subscribeT1UpCost(
        up->level(), params[WizardParams::MultiUpCost],
        [](const Number& lvl) { return 175 * (1.3 ^ lvl); }));
    mParamSubs.push_back(params[WizardParams::MultiUp].subscribeTo(
        up->level(), [](const Number& lvl) { return .05 * lvl; }));
    mMultiUp = mUpgrades->subscribe(up);
}
void Wizard::setParamTriggers() {
    ParameterSystem::Params<WIZARD> params;
    ParameterSystem::Params<CRYSTAL> crysParams;
    ParameterSystem::Params<CATALYST> catParams;
    ParameterSystem::Params<TIME_WIZARD> timeParams;
    ParameterSystem::States states;

    mParamSubs.push_back(params[WizardParams::Power].subscribeTo(
        {params[WizardParams::BasePower], params[WizardParams::PowerUp],
         params[WizardParams::Speed], params[WizardParams::PowerWizEffect],
         crysParams[CrystalParams::MagicEffect],
         crysParams[CrystalParams::WizardCntEffect],
         catParams[CatalystParams::MagicEffect],
         catParams[CatalystParams::FBCntEffect]},
        {}, [this]() { return calcPower(); }));

    mParamSubs.push_back(params[WizardParams::Speed].subscribeTo(
        {params[WizardParams::BaseSpeed],
         timeParams[TimeWizardParams::SpeedEffect]},
        {states[State::WizBoosted]}, [this]() { return calcSpeed(); }));

    mParamSubs.push_back(params[WizardParams::FBSpeed].subscribeTo(
        {params[WizardParams::BaseFBSpeed],
         timeParams[TimeWizardParams::FBSpeedUp]},
        {}, [this]() { return calcFBSpeed(); }));

    mParamSubs.push_back(params[WizardParams::FBSpeedEffect].subscribeTo(
        {params[WizardParams::FBSpeed]}, {},
        [this]() { return calcFBSpeedEffect(); }));

    mParamSubs.push_back(
        params[WizardParams::Speed].subscribe([this]() { calcTimer(); }));

    mParamSubs.push_back(params[WizardParams::Crit].subscribeTo(
        {params[WizardParams::BaseCrit], params[WizardParams::CritUp],
         params[WizardParams::RoboCritUp]},
        {states[State::BoughtRoboWizCritUp]}, [this]() { return calcCrit(); }));

    mParamSubs.push_back(params[WizardParams::CritSpread].subscribeTo(
        {params[WizardParams::BaseCritSpread],
         params[WizardParams::CritSpreadUp]},
        {}, [this]() { return calcCritSpread(); }));

    mParamSubs.push_back(states[State::TimeWizFrozen].subscribe(
        [this](bool val) { onTimeFreeze(val); }));

    mParamSubs.push_back(params[WizardParams::PowerUpMaxLvl].subscribeTo(
        states[State::BoughtFirstT1], [this](bool val) {
            if (val) {
                showStar();
            }
            return val ? 10 : 5;
        }));

    // Upgrade unlock constraints
    mParamSubs.push_back(states[State::BoughtPowerWizard].subscribe(
        [this](bool val) { mCritUp->setActive(val); }));
    mParamSubs.push_back(states[State::BoughtTimeWizard].subscribe(
        [this](bool val) { mMultiUp->setActive(val); }));
    mParamSubs.push_back(ParameterSystem::subscribe(
        {}, {states[State::BoughtCatalyst]}, [this]() {
            ParameterSystem::States states;
            mTargetUp->setActive(states[State::BoughtCatalyst].get());
        }));
    mParamSubs.push_back(states[State::BoughtRoboWizCritUp].subscribe(
        [this](bool bought) { mRoboCritUp->setActive(bought); }));
}

void Wizard::onRender(SDL_Renderer* r) {
    TextureBuilder tex;

    if (mPowWizTimerSub &&
        mPowWizTimerSub->get<TimerObservable::DATA>().isActive()) {
        mPowBkgrnd.setDest(mPos->rect);
        tex.draw(mPowBkgrnd);
    }

    if (mFreezeFireball) {
        mFreezeFireball->draw(tex);
    }

    WizardBase::onRender(r);
}

void Wizard::onHide(bool hide) {
    WizardBase::onHide(hide);

    mFireballs->clear();
}

void Wizard::onTargetHide(WizardId id, bool hide) {
    if (hide) {
        mFireballs->remove(id);
        if (id == mTarget) {
            mTarget = CRYSTAL;
        }
    }
}

void Wizard::onT1Reset() {
    mFireballs->clear();
    mPowWizBoosts.clear();

    if (mFireballTimerSub) {
        mFireballTimerSub->get<TimerObservable::DATA>().reset();
    }
    mPowWizTimerSub.reset();
}

bool Wizard::onTimer(Timer& timer) {
    ParameterSystem::Params<WIZARD> params;

    shootFireball(newFireballData());
    float Multi = params[WizardParams::MultiUp].get().toFloat();
    if (rDist(gen) < Multi) {
        shootFireball(
            newFireballData(),
            SDL_FPoint{(rDist(gen) - .5f) * mPos->rect.w() + mPos->rect.cX(),
                       (rDist(gen) - .5f) * mPos->rect.h() + mPos->rect.cY()});
    }
    return true;
}

void Wizard::onPowFireballHit(const PowerWizard::Fireball& fireball) {
    Number power = fireball.getPower(), duration = fireball.getDuration();
    for (auto it = mPowWizBoosts.begin(), end = mPowWizBoosts.end();
         duration > 0 && it != end; ++it) {
        if (power >= it->first) {  // Replace current
            if (power == it->first && duration <= it->first) {  // No change
                break;
            }
            if (duration < it->second) {  // Reduce current
                mPowWizBoosts.insert(
                    it + 1, std::make_pair(it->first, it->second - duration));
            } else {  // Remove current and
                      // update future
                Number durationLeft = duration - it->second;
                for (auto it2 = it + 1; durationLeft > 0 && it2 != end; ++it2) {
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

    ParameterSystem::Param(State::WizBoosted).set(true);
    ParameterSystem::Param<WIZARD>(WizardParams::PowerWizEffect)
        .set(mPowWizBoosts.front().first);
    mPowWizTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            bool boosted = onPowWizTimer(t);
            ParameterSystem::Param(State::WizBoosted).set(boosted);
            return boosted;
        },
        [this](Time dt, Timer& t) { onPowWizTimerUpdate(dt, t); },
        Timer(mPowWizBoosts.front().second.toFloat()));
}

void Wizard::onGlobHit() {
    auto data = newFireballData();
    data.poisoned = true;
    shootFireball(data);
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

void Wizard::onTimeFreeze(bool frozen) {
    if (!frozen && mFreezeFireball) {
        Number freezeEffect =
            ParameterSystem::Param<TIME_WIZARD>(TimeWizardParams::FreezeEffect)
                .get();
        mFreezeFireball->applyTimeEffect(freezeEffect);
        mFireballs->push_back(std::move(mFreezeFireball));
    }
}

void Wizard::onTimeWarp() {
    Number effect =
        ParameterSystem::Param<TIME_WIZARD>(TimeWizardParams::TimeWarpEffect)
            .get();
    for (auto& fireball : *mFireballs) {
        fireball.setSpeed(fireball.getSpeed() * 10);
        fireball.setPower(fireball.getPower() * effect);
    }
}

void Wizard::shootFireball(const Fireball::Data& data) {
    if (!ParameterSystem::Param(State::TimeWizFrozen).get()) {
        mFireballs->push_back(ComponentFactory<Fireball>::New(
            SDL_FPoint{mPos->rect.cX(), mPos->rect.cY()}, mTarget, data));
    } else {
        if (!mFreezeFireball) {
            mFreezeFireball = ComponentFactory<Fireball>::New(
                SDL_FPoint{mPos->rect.cX(), mPos->rect.cY()}, mTarget, data);
        } else {
            mFreezeFireball->addFireball(data);
        }
    }
}

void Wizard::shootFireball(const Fireball::Data& data, SDL_FPoint launch) {
    size_t size = mFireballs->size();
    shootFireball(data);
    if (size != mFireballs->size()) {
        mFireballs->back().launch(launch);
    }
}

Number Wizard::calcPower() {
    ParameterSystem::Params<WIZARD> params;
    ParameterSystem::Params<CRYSTAL> crysParams;
    ParameterSystem::Params<CATALYST> catParams;

    return (params[WizardParams::BasePower].get() +
            params[WizardParams::PowerUp].get()) *
           params[WizardParams::PowerWizEffect].get() *
           crysParams[CrystalParams::MagicEffect].get() *
           crysParams[CrystalParams::WizardCntEffect].get() *
           catParams[CatalystParams::MagicEffect].get() *
           catParams[CatalystParams::FBCntEffect].get() *
           max(1, params[WizardParams::Speed].get() * 16 / 1000);
}

Number Wizard::calcSpeed() {
    ParameterSystem::Params<WIZARD> params;
    ParameterSystem::Params<TIME_WIZARD> timeParams;

    Number speed = params[WizardParams::BaseSpeed].get() *
                   timeParams[TimeWizardParams::SpeedEffect].get();
    return speed;
}

Number Wizard::calcFBSpeed() {
    ParameterSystem::Params<WIZARD> params;
    ParameterSystem::Params<TIME_WIZARD> timeParams;

    return params[WizardParams::BaseFBSpeed].get() *
           timeParams[TimeWizardParams::FBSpeedUp].get();
}

Number Wizard::calcFBSpeedEffect() {
    ParameterSystem::Params<WIZARD> params;
    Number fbSpeed = max(params[WizardParams::FBSpeed].get(), 1);

    Number twoFbSpeed = 2 * fbSpeed;

    return fbSpeed * (twoFbSpeed ^ (twoFbSpeed - 2));
}

void Wizard::calcTimer() {
    ParameterSystem::Params<WIZARD> params;

    Timer& timer = mFireballTimerSub->get<TimerObservable::DATA>();
    float div = params[WizardParams::Speed].get().toFloat();
    if (div <= 0) {
        div = 1;
    }
    float prevLen = timer.length;
    timer.length = fmax(1000 / div, 16);
    timer.timer *= timer.length / prevLen;
}

Number Wizard::calcCrit() {
    ParameterSystem::Params<WIZARD> params;
    ParameterSystem::States states;
    Number crit = (params[WizardParams::BaseCrit].get() +
                   params[WizardParams::CritUp].get()) *
                  params[WizardParams::RoboCritUp].get();
    if (states[State::BoughtRoboWizCritUp].get()) {
        crit.powTen();
    }
    return crit;
}

Number Wizard::calcCritSpread() {
    ParameterSystem::Params<WIZARD> params;
    return params[WizardParams::BaseCritSpread].get() +
           params[WizardParams::CritSpreadUp].get();
}

Fireball::Data Wizard::newFireballData() {
    ParameterSystem::Params<WIZARD> params;
    ParameterSystem::States states;

    Number power = params[WizardParams::Power].get() *
                   params[WizardParams::FBSpeedEffect].get();
    float speed = params[WizardParams::FBSpeed].get().toFloat();

    if (params[WizardParams::CritUpLvl].get() == 0) {
        return {power, 1, speed};
    }

    float frac = rDist(gen);
    if (frac == 0) {
        frac = std::numeric_limits<float>::min();
    }
    frac = (frac ^ params[WizardParams::CritSpread].get()).toFloat() + .5;
    return {power * params[WizardParams::Crit].get() * frac, powf(frac, .25),
            speed, !mPowWizBoosts.empty()};
}

void Wizard::setPos(float x, float y) {
    WizardBase::setPos(x, y);
    if (mFreezeFireball) {
        mFreezeFireball->setPos(mPos->rect.cX(), mPos->rect.cY());
    }
}
}  // namespace Wizard