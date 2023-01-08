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
    Params params;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(mId);
    dUp->setEffects({params[Param::Power], params[Param::FBSpeed],
                     params[Param::FBSpeedEffect], params[Param::Speed],
                     params[Param::Crit]},
                    {}, []() -> TextUpdateData {
                        Params params;
                        std::stringstream ss;
                        std::vector<RenderTextureCPtr> imgs;
                        ss << "Power: "
                           << UpgradeDefaults::MultiplicativeEffectText(
                                  params[Param::Power].get())
                           << "\nFire Rate: " << params[Param::Speed].get()
                           << "/s\n{i} Speed: "
                           << UpgradeDefaults::MultiplicativeEffectText(
                                  params[Param::FBSpeed].get())
                           << ", {b}Power : "
                           << UpgradeDefaults::MultiplicativeEffectText(
                                  params[Param::FBSpeedEffect].get())
                           << "\nCrit Power: " << params[Param::Crit].get();
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
    UpgradePtr up = std::make_shared<Upgrade>(params[Param::PowerUpLvl],
                                              params[Param::PowerUpMaxLvl]);
    up->setImage(Constants::POWER_UP_IMG);
    up->setDescription({"Increase base power by 1"});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC, params[Param::PowerUpCost]);
    up->setEffects(params[Param::PowerUp], UpgradeDefaults::AdditiveEffect);
    mParamSubs.push_back(params[Param::PowerUpCost].subscribeTo(
        up->level(), [](const Number& lvl) { return 25 * (1.55 ^ lvl); }));
    mParamSubs.push_back(params[Param::PowerUp].subscribeTo(
        up->level(),
        [](const Number& lvl) { return Number::min(lvl, 5) + 2 * Number::max(lvl - 5, 0); }));
    mPowerUp = mUpgrades->subscribe(up);

    // Crit Upgrade
    up = std::make_shared<Upgrade>(params[Param::CritUpLvl], 10);
    up->setImage(Constants::CRIT_UP_IMG);
    up->setDescription(
        {"Multiplies critical hit power *1.1 and increases chance for "
         "higher crits"});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC, params[Param::CritUpCost]);
    up->setEffects({params[Param::CritUp], params[Param::CritSpreadUp]}, {},
                   []() -> TextUpdateData {
                       Params params;
                       std::stringstream ss;
                       ss << "Crit: +" << params[Param::CritUp].get()
                          << " | Spread: *"
                          << params[Param::CritSpreadUp].get();
                       return {ss.str()};
                   });
    mParamSubs.push_back(UpgradeDefaults::subscribeT1UpCost(
        up->level(), params[Param::CritUpCost],
        [](const Number& lvl) { return 200 * (1.5 ^ lvl); }));
    mParamSubs.push_back(params[Param::CritUp].subscribeTo(
        up->level(), [](const Number& lvl) { return (1.1 ^ lvl) - 1; }));
    mParamSubs.push_back(params[Param::CritSpreadUp].subscribeTo(
        up->level(), [](const Number& lvl) { return .95 ^ lvl; }));
    mCritUp = mUpgrades->subscribe(up);

    // Robo Crit Upgrade
    up = std::make_shared<Upgrade>(params[Param::RoboCritUpLvl], 10);
    up->setImage("");
    up->setDescription({"Multiplies critical hit power *1.15"});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC, params[Param::RoboCritUpCost]);
    up->setEffects(params[Param::RoboCritUp],
                   UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(params[Param::RoboCritUpCost].subscribeTo(
        up->level(),
        [](const Number& lvl) { return Number(1, 20) * (11 ^ lvl); }));
    mParamSubs.push_back(params[Param::RoboCritUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1.15 ^ lvl; }));
    mRoboCritUp = mUpgrades->subscribe(up);

    // Multi Upgrade
    up = std::make_shared<Upgrade>(params[Param::MultiUpLvl], 20);
    up->setImage(Constants::MULTI_UP_IMG);
    up->setDescription({"Increase double {i} chance by +5%",
                        {IconSystem::Get(Constants::FB_IMG())}});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC, params[Param::MultiUpCost]);
    up->setEffects(params[Param::MultiUp], UpgradeDefaults::PercentEffect);
    mParamSubs.push_back(UpgradeDefaults::subscribeT1UpCost(
        up->level(), params[Param::MultiUpCost],
        [](const Number& lvl) { return 175 * (1.3 ^ lvl); }));
    mParamSubs.push_back(params[Param::MultiUp].subscribeTo(
        up->level(), [](const Number& lvl) { return .05 * lvl; }));
    mMultiUp = mUpgrades->subscribe(up);
}
void Wizard::setParamTriggers() {
    Params params;
    Crystal::Params cryParams;
    Catalyst::Params catParams;
    TimeWizard::Params timeParams;
    RobotWizard::Params roboParams;

    mParamSubs.push_back(params[Param::Power].subscribeTo(
        {params[Param::BasePower], params[Param::PowerUp], params[Param::Speed],
         params[Param::PowerWizEffect], cryParams[Crystal::Param::MagicEffect],
         cryParams[Crystal::Param::WizardCntEffect],
         catParams[Catalyst::Param::MagicEffect],
         catParams[Catalyst::Param::FBCntEffect],
         roboParams[RobotWizard::Param::ShardPowerUp]},
        {}, [this]() { return calcPower(); }));

    mParamSubs.push_back(params[Param::Speed].subscribeTo(
        {params[Param::BaseSpeed], timeParams[TimeWizard::Param::SpeedEffect]},
        {params[Param::Boosted]}, [this]() { return calcSpeed(); }));

    mParamSubs.push_back(params[Param::FBSpeed].subscribeTo(
        {params[Param::BaseFBSpeed], timeParams[TimeWizard::Param::FBSpeedUp]},
        {}, [this]() { return calcFBSpeed(); }));

    mParamSubs.push_back(params[Param::FBSpeedEffect].subscribeTo(
        {params[Param::FBSpeed]}, {},
        [this]() { return calcFBSpeedEffect(); }));

    mParamSubs.push_back(
        params[Param::Speed].subscribe([this]() { calcTimer(); }));

    mParamSubs.push_back(params[Param::Crit].subscribeTo(
        {params[Param::BaseCrit], params[Param::CritUp],
         params[Param::RoboCritUp]},
        {roboParams[RobotWizard::Param::BoughtWizCritUp]},
        [this]() { return calcCrit(); }));

    mParamSubs.push_back(params[Param::CritSpread].subscribeTo(
        {params[Param::BaseCritSpread], params[Param::CritSpreadUp]}, {},
        [this]() { return calcCritSpread(); }));

    mParamSubs.push_back(timeParams[TimeWizard::Param::Frozen].subscribe(
        [this](bool val) { onTimeFreeze(val); }));

    mParamSubs.push_back(params[Param::PowerUpMaxLvl].subscribeTo(
        cryParams[Crystal::Param::BoughtFirstT1], [this](bool val) {
            if (val) {
                showStar();
            }
            return val ? 10 : 5;
        }));

    // Upgrade unlock constraints
    mParamSubs.push_back(cryParams[Crystal::Param::BoughtPowerWizard].subscribe(
        [this](bool val) { mCritUp->setActive(val); }));
    mParamSubs.push_back(cryParams[Crystal::Param::BoughtTimeWizard].subscribe(
        [this](bool val) { mMultiUp->setActive(val); }));
    mParamSubs.push_back(ParameterSystem::subscribe(
        {}, {cryParams[Crystal::Param::BoughtCatalyst]}, [this, cryParams]() {
            mTargetUp->setActive(
                cryParams[Crystal::Param::BoughtCatalyst].get());
        }));
    mParamSubs.push_back(
        roboParams[RobotWizard::Param::BoughtWizCritUp].subscribe(
            [this](bool bought) { mRoboCritUp->setActive(bought); }));
}

void Wizard::onRender(SDL_Renderer* r) {
    TextureBuilder tex;

    if (!mPowWizBoosts.empty()) {
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
    Params params;

    shootFireball(newFireballData());
    float Multi = params[Param::MultiUp].get().toFloat();
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

    Params::get(Param::Boosted).set(true);
    Params::get(Param::PowerWizEffect).set(mPowWizBoosts.front().first);
    mPowWizTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            bool boosted = onPowWizTimer(t);
            Params::get(Param::Boosted).set(boosted);
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
    auto powerWizEffect = Params::get(Param::PowerWizEffect);
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
            TimeWizard::Params::get(TimeWizard::Param::FreezeEffect).get();
        mFreezeFireball->applyTimeEffect(freezeEffect);
        mFireballs->push_back(std::move(mFreezeFireball));
    }
}

void Wizard::onTimeWarp() {
    Number effect =
        TimeWizard::Params::get(TimeWizard::Param::TimeWarpEffect).get();
    for (auto& fireball : *mFireballs) {
        fireball.setSpeedFactor(fireball.getSpeedFactor() * 10);
        fireball.setPower(fireball.getPower() * effect);
    }
}

void Wizard::shootFireball(const Fireball::Data& data) {
    if (!TimeWizard::Params::get(TimeWizard::Param::Frozen).get()) {
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
    Params params;
    Crystal::Params crysParams;
    Catalyst::Params catParams;
    RobotWizard::Params roboParams;

    return (params[Param::BasePower].get() + params[Param::PowerUp].get()) *
           params[Param::PowerWizEffect].get() *
           crysParams[Crystal::Param::MagicEffect].get() *
           crysParams[Crystal::Param::WizardCntEffect].get() *
           catParams[Catalyst::Param::MagicEffect].get() *
           catParams[Catalyst::Param::FBCntEffect].get() *
           roboParams[RobotWizard::Param::ShardPowerUp].get() *
           Number::max(1, params[Param::Speed].get() * 16 / 1000);
}

Number Wizard::calcSpeed() {
    Params params;
    TimeWizard::Params timeParams;

    Number speed = params[Param::BaseSpeed].get() *
                   timeParams[TimeWizard::Param::SpeedEffect].get();
    return speed;
}

Number Wizard::calcFBSpeed() {
    Params params;
    TimeWizard::Params timeParams;

    return params[Param::BaseFBSpeed].get() *
           timeParams[TimeWizard::Param::FBSpeedUp].get();
}

Number Wizard::calcFBSpeedEffect() {
    Params params;
    Number fbSpeed = Number::max(params[Param::FBSpeed].get(), 1);

    Number twoFbSpeed = 2 * fbSpeed;

    return fbSpeed * (twoFbSpeed ^ (twoFbSpeed - 2));
}

void Wizard::calcTimer() {
    Params params;

    Timer& timer = mFireballTimerSub->get<TimerObservable::DATA>();
    float div = params[Param::Speed].get().toFloat();
    if (div <= 0) {
        div = 1;
    }
    float prevLen = timer.length;
    timer.length = fmax(1000 / div, 16);
    timer.timer *= timer.length / prevLen;
}

Number Wizard::calcCrit() {
    Params params;
    Number crit =
        (params[Param::BaseCrit].get() + params[Param::CritUp].get()) *
        params[Param::RoboCritUp].get();
    if (RobotWizard::Params::get(RobotWizard::Param::BoughtWizCritUp).get()) {
        crit.powTen();
    }
    return crit;
}

Number Wizard::calcCritSpread() {
    Params params;
    return params[Param::BaseCritSpread].get() +
           params[Param::CritSpreadUp].get();
}

Fireball::Data Wizard::newFireballData() {
    Params params;

    Fireball::Data data;

    data.boosted = !mPowWizBoosts.empty();
    data.power =
        params[Param::Power].get() * params[Param::FBSpeedEffect].get();
    data.speed = params[Param::FBSpeed].get().toFloat();

    if (params[Param::CritUpLvl].get() > 0) {
        float frac = rDist(gen);
        if (frac == 0) {
            frac = std::numeric_limits<float>::min();
        }
        frac = (frac ^ params[Param::CritSpread].get()).toFloat() + .5;
        Fireball::Data data;
        data.power *= params[Param::Crit].get() * frac;
        data.sizeFactor = powf(frac, .25);
    }
    return data;
}

void Wizard::setPos(float x, float y) {
    WizardBase::setPos(x, y);
    if (mFreezeFireball) {
        mFreezeFireball->setPos(mPos->rect.cX(), mPos->rect.cY());
    }
}
}  // namespace Wizard