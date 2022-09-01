#include "PowerWizard.h"

// PowerWizard
const AnimationData PowerWizard::IMG{"res/wizards/power_wizard_ss.png", 8, 150};

const std::string PowerWizard::POWER_UP_IMG =
    "res/upgrades/power_fireball_upgrade.png";

void PowerWizard::setDefaults() {
    using WizardSystem::Event;

    ParameterSystem::Params<POWER_WIZARD> params;

    params[PowerWizardParams::BasePower]->init(5);
    params[PowerWizardParams::BaseSpeed]->init(.25);
    params[PowerWizardParams::BaseFBSpeed]->init(.75);

    params[PowerWizardParams::PowerUpLvl]->init(Event::ResetT1);
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
    mAnimTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            mImg.nextFrame();
            WizardSystem::GetWizardImageObservable()->next(mId, mImg);
            return true;
        },
        IMG);
    mT1ResetSub = WizardSystem::GetWizardEventObservable()->subscribe(
        [this]() { onT1Reset(); }, WizardSystem::Event::ResetT1);
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
    up->setImage(POWER_UP_IMG);
    up->setDescription({"Increase power effect by *1.15"});
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
    Number speedEffect = params[PowerWizardParams::FBSpeedEffect].get();

    PowerWizFireball::Data data;
    switch (target) {
        case WIZARD:
            data.power = params[PowerWizardParams::Power].get() * speedEffect;
            break;
        case CRYSTAL:
            data.power =
                params[PowerWizardParams::FireRingEffect].get() * speedEffect;
            break;
    };
    data.duration = params[PowerWizardParams::Duration].get();
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