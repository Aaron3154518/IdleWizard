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

std::unique_ptr<Fireball>& PowerWizard::shootFireball() {
    WizardId target = rDist(gen) < .5 ? WIZARD : CRYSTAL;
    mFireballs.push_back(std::move(ComponentFactory<Fireball>::New(
        SDL_FPoint{mPos->rect.cX(), mPos->rect.cY()}, mId, target,
        ParameterSystem::Get()->get<POWER_WIZARD>(PowerWizardParams::Power),
        FIREBALL_IMG)));
    return mFireballs.back();
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
