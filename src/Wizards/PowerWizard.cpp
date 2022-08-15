#include "PowerWizard.h"

// PowerWizard
const std::string PowerWizard::FIREBALL_IMG = "res/projectiles/fireball2.png";

PowerWizard::PowerWizard() : WizardBase(POWER_WIZARD) {
    auto params = ParameterSystem::Get();
    params->set<POWER_WIZARD>(PowerWizardParams::BasePower, 5);
    params->set<POWER_WIZARD>(PowerWizardParams::BaseSpeed, .25);
    params->set<POWER_WIZARD>(PowerWizardParams::FireRingEffect, 1);
    params->set<POWER_WIZARD>(PowerWizardParams::Duration, 1000);
}

void PowerWizard::init() {
    WizardBase::init();

    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&PowerWizard::onRender, this, std::placeholders::_1),
            mPos);
    mFireballTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            std::bind(&PowerWizard::onTimer, this), Timer(1000));
    mFreezeSub = TimeSystem::GetFreezeObservable()->subscribe(
        std::bind(&PowerWizard::onFreeze, this, std::placeholders::_1),
        std::bind(&PowerWizard::onUnfreeze, this, std::placeholders::_1));

    // Power Display
    UpgradePtr up = std::make_shared<Upgrade>();
    up->setMaxLevel(0)
        .setEffectSource<POWER_WIZARD, PowerWizardParams::Power>(
            Upgrade::Defaults::MultiplicativeEffect)
        .setImg(WIZ_IMGS.at(mId))
        .setDescription("Power");
    mPowerDisplay = mUpgrades->subscribe(up);

    auto params = ParameterSystem::Get();
    mParamSubs.push_back(
        params->subscribe<Keys<POWER_WIZARD, PowerWizardParams::BasePower,
                               PowerWizardParams::Speed>>(
            std::bind(&PowerWizard::calcPower, this)));

    mParamSubs.push_back(
        params->subscribe<Keys<POWER_WIZARD, PowerWizardParams::BaseSpeed>,
                          Keys<TIME_WIZARD, TimeWizardParams::SpeedEffect>>(
            std::bind(&PowerWizard::calcSpeed, this)));
    mParamSubs.push_back(
        params->subscribe<Keys<POWER_WIZARD, PowerWizardParams::Power>>(
            std::bind(&PowerWizard::calcFireRingEffect, this)));
    mParamSubs.push_back(params->subscribe<POWER_WIZARD>(
        PowerWizardParams::Speed, std::bind(&PowerWizard::calcTimer, this)));
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

bool PowerWizard::onTimer() {
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
                FireballPtr& fireball = mFireballs.back();
                fireball->getValue(PowerWizardParams::Duration) ^=
                    ParameterSystem::Get()->get<TIME_WIZARD>(
                        TimeWizardParams::FreezeEffect);
                std::cerr << "Do pretty graphics stuff" << std::endl;
            }
            break;
    }
}

void PowerWizard::shootFireball(SDL_FPoint launch) {
    bool frozen = TimeSystem::Frozen(TimeSystem::FreezeType::TIME_WIZARD);
    if (!frozen || mFireballFreezeCnt == 0) {
        WizardId target = rDist(gen) < .5 ? WIZARD : CRYSTAL;
        mFireballs.push_back(std::move(ComponentFactory<Fireball>::New(
            SDL_FPoint{mPos->rect.cX(), mPos->rect.cY()}, mId, target,
            FIREBALL_IMG,
            NumberMap<int>{{PowerWizardParams::Power,
                            ParameterSystem::Get()->get<POWER_WIZARD>(
                                PowerWizardParams::Power)},
                           {PowerWizardParams::Duration,
                            ParameterSystem::Get()->get<POWER_WIZARD>(
                                PowerWizardParams::Duration)}})));
        if (!frozen) {
            mFireballs.back()->launch(launch);
        }
    }
    if (frozen) {
        mFireballFreezeCnt++;
        if (mFireballFreezeCnt > 1) {
            mFireballs.back()->getValue(PowerWizardParams::Duration) +=
                ParameterSystem::Get()->get<POWER_WIZARD>(
                    PowerWizardParams::Duration);
        }
        mFireballs.back()->setSize(
            fmin(pow(mFireballFreezeCnt, 1.0 / 3.0), 10));
    }
}

void PowerWizard::calcPower() {
    auto params = ParameterSystem::Get();
    Number power =
        params->get<POWER_WIZARD>(PowerWizardParams::BasePower) *
        max(1, params->get<POWER_WIZARD>(PowerWizardParams::Speed) * 16 / 1000);
    params->set<POWER_WIZARD>(PowerWizardParams::Power, power);
}

void PowerWizard::calcSpeed() {
    auto params = ParameterSystem::Get();
    Number speed = params->get<POWER_WIZARD>(PowerWizardParams::BaseSpeed) *
                   params->get<TIME_WIZARD>(TimeWizardParams::SpeedEffect);
    params->set<POWER_WIZARD>(PowerWizardParams::Speed, speed);
}

void PowerWizard::calcTimer() {
    Timer& timer = mFireballTimerSub->get<TimerObservable::DATA>();
    float div = ParameterSystem::Get()
                    ->get<POWER_WIZARD>(PowerWizardParams::Speed)
                    .toFloat();
    if (div <= 0) {
        div = 1;
    }
    timer.length = fmax(1000 / div, 16);
}

void PowerWizard::calcFireRingEffect() {
    auto params = ParameterSystem::Get();
    Number effect =
        (params->get<POWER_WIZARD>(PowerWizardParams::Power).logTenCopy() + 1)
            .sqrt();
    params->set<POWER_WIZARD>(PowerWizardParams::FireRingEffect, effect);
}

void PowerWizard::setPos(float x, float y) {
    WizardBase::setPos(x, y);
    if (TimeSystem::Frozen(TimeSystem::FreezeType::TIME_WIZARD) &&
        mFireballFreezeCnt > 0) {
        mFireballs.back()->setPos(mPos->rect.cX(), mPos->rect.cY());
    }
}
