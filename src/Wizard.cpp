#include "Wizard.h"

// Wizard
const std::string Wizard::POWER_UP_IMG = "res/upgrades/fireball_upgrade.png";
const std::string Wizard::SPEED_UP_IMG = "res/upgrades/speed_upgrade.png";

Wizard::Wizard() : WizardBase(WizardId::WIZARD) {
    auto params = ServiceSystem::Get<ParameterService, ParameterMap>();
    params->set<WizardId::WIZARD>(WizardParams::PowerUpgrade, 0);
    params->set<WizardId::WIZARD>(WizardParams::Speed, 1);
}

void Wizard::init() {
    WizardBase::init();

    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&Wizard::onRender, this, std::placeholders::_1), mPos);
    mMouseSub = ServiceSystem::Get<MouseService, MouseObservable>()->subscribe(
        std::bind(&Wizard::onClick, this, std::placeholders::_1,
                  std::placeholders::_2),
        mPos);
    mTimerSub = ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
        std::bind(&Wizard::onTimer, this), Timer(1000));

    mParamSubs.push_back(
        ServiceSystem::Get<ParameterService, ParameterMap>()
            ->subscribe<WizardId::WIZARD, WizardId::CRYSTAL,
                        WizardId::CATALYST>(std::bind(&Wizard::calcPower, this),
                                            {
                                                WizardParams::PowerUpgrade,
                                            },
                                            {
                                                CrystalParams::MagicEffect,
                                            },
                                            {
                                                CatalystParams::MagicEffect,
                                            }));

    // Target Upgrade
    Upgrade up{-1};
    up.setImg(WIZ_IMGS.at(mTarget));
    up.setDescription("Change the Wizard's target");
    mTargetUp = mUpgrades->subscribe(
        [this](Upgrade& u) {
            u.mLevel = u.mLevel % 2;
            mTarget = u.mLevel == 0 ? WizardId::CRYSTAL : WizardId::CATALYST;
            u.mEffect = "Target: " + WIZ_NAMES.at(mTarget);
        },
        [this](Upgrade& u) { return 0; }, [this](Upgrade& u) { return true; },
        up);

    // Power Upgrade
    up = {1};
    up.setImg(POWER_UP_IMG);
    up.setDescription("Increase Wizard base power by 1");
    mPowerUp = mUpgrades->subscribe(
        [this](Upgrade& u) {
            ServiceSystem::Get<ParameterService, ParameterMap>()
                ->set<WizardId::WIZARD>(WizardParams::PowerUpgrade, u.mLevel);
            u.mEffect = "+" + std::to_string(u.mLevel);
        },
        [this](Upgrade& u) { return 10; }, [this](Upgrade& u) { return true; },
        up);

    // Speed Upgrade
    up = {5};
    up.setImg(SPEED_UP_IMG);
    up.setDescription("Increase Wizard fire rate by 33%");
    mSpeedUp = mUpgrades->subscribe(
        [this](Upgrade& u) {
            Timer& timer = mTimerSub->get<TimerObservable::DATA>();
            timer.length = 1000 * pow(.75, u.mLevel);
            Number speed = Number(1000) / timer.length;
            ServiceSystem::Get<ParameterService, ParameterMap>()
                ->set<WizardId::WIZARD>(WizardParams::Speed, speed);
            u.mEffect = speed.toString() + "x";
        },
        [this](Upgrade& u) { return (Number(1.5) ^ u.mLevel) * 100; },
        [this](Upgrade& u) { return true; }, up);
}

void Wizard::onRender(SDL_Renderer* r) {
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

void Wizard::onClick(Event::MouseButton b, bool clicked) {
    if (clicked) {
        ServiceSystem::Get<UpgradeService, UpgradeListObservable>()->next(
            mUpgrades);
    }
}

bool Wizard::onTimer() {
    shootFireball();
    return true;
}

void Wizard::shootFireball() {
    mFireballs.push_back(std::move(ComponentFactory<Fireball>::New(
        mPos->rect.cX(), mPos->rect.cY(), mId, mTarget,
        ServiceSystem::Get<ParameterService, ParameterMap>()
            ->get<WizardId::WIZARD>(WizardParams::Power))));
}

void Wizard::calcPower() {
    auto params = ServiceSystem::Get<ParameterService, ParameterMap>();
    Number power =
        (1 + params->get<WizardId::WIZARD>(WizardParams::PowerUpgrade)) *
        params->get<WizardId::CRYSTAL>(CrystalParams::MagicEffect) *
        params->get<WizardId::CATALYST>(CatalystParams::MagicEffect);
    params->set<WizardId::WIZARD>(WizardParams::Power, power);
}

// Crystal
Crystal::Crystal() : WizardBase(WizardId::CRYSTAL) {
    auto params = ServiceSystem::Get<ParameterService, ParameterMap>();
    params->set<WizardId::CRYSTAL>(CrystalParams::Magic, 0);
}

void Crystal::init() {
    WizardBase::init();

    auto params = ServiceSystem::Get<ParameterService, ParameterMap>();
    mMagicText.tData.font = AssetManager::getFont(FONT);
    mMagicText.tData.text =
        params->get<WizardId::CRYSTAL>(CrystalParams::Magic).toString();
    mMagicText.renderText();

    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&Crystal::onRender, this, std::placeholders::_1), mPos);
    mTargetSub =
        ServiceSystem::Get<FireballService, TargetObservable>()->subscribe(
            std::bind(&Crystal::onHit, this, std::placeholders::_1,
                      std::placeholders::_2),
            mId);

    mParamSubs.push_back(params->subscribe<WizardId::CRYSTAL>(
        CrystalParams::Magic, std::bind(&Crystal::calcMagicEffect, this)));
}

void Crystal::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    mMagicText.dest = Rect(mPos->rect.x(), mPos->rect.y(), mPos->rect.w(), 0);
    mMagicText.dest.setHeight(FONT.h, Rect::Align::BOT_RIGHT);
    mMagicText.fitToTexture();
    TextureBuilder().draw(mMagicText);
}

void Crystal::onHit(WizardId src, Number val) {
    switch (src) {
        case WizardId::WIZARD:
            auto params = ServiceSystem::Get<ParameterService, ParameterMap>();
            Number magic =
                params->get<WizardId::CRYSTAL>(CrystalParams::Magic) + val;
            params->set<WizardId::CRYSTAL>(CrystalParams::Magic, magic);
            mMagicText.tData.text = magic.toString();
            mMagicText.renderText();
            break;
    }
}

void Crystal::calcMagicEffect() {
    auto params = ServiceSystem::Get<ParameterService, ParameterMap>();
    Number effect =
        (params->get<WizardId::CRYSTAL>(CrystalParams::Magic) + 1).logTen() + 1;
    params->set<WizardId::CRYSTAL>(CrystalParams::MagicEffect, effect);
}

// Catalyst
Catalyst::Catalyst() : WizardBase(WizardId::CATALYST) {
    auto params = ServiceSystem::Get<ParameterService, ParameterMap>();
    params->set<WizardId::CATALYST>(CatalystParams::Magic, 0);
    params->set<WizardId::CATALYST>(CatalystParams::Capacity, 100);
}

void Catalyst::init() {
    WizardBase::init();

    auto params = ServiceSystem::Get<ParameterService, ParameterMap>();
    mMagicText.tData.font = AssetManager::getFont(FONT);
    mMagicText.tData.text =
        params->get<WizardId::CATALYST>(CatalystParams::Magic).toString() +
        "/" +
        params->get<WizardId::CATALYST>(CatalystParams::Capacity).toString();
    mMagicText.renderText();

    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&Catalyst::onRender, this, std::placeholders::_1), mPos);
    mTargetSub =
        ServiceSystem::Get<FireballService, TargetObservable>()->subscribe(
            std::bind(&Catalyst::onHit, this, std::placeholders::_1,
                      std::placeholders::_2),
            mId);

    mParamSubs.push_back(params->subscribe<WizardId::CATALYST>(
        CatalystParams::Magic, std::bind(&Catalyst::calcMagicEffect, this)));
}

void Catalyst::onHit(WizardId src, Number val) {
    switch (src) {
        case WizardId::WIZARD:
            auto params = ServiceSystem::Get<ParameterService, ParameterMap>();
            Number cap =
                params->get<WizardId::CATALYST>(CatalystParams::Capacity);
            Number magic = max(
                min(params->get<WizardId::CRYSTAL>(CrystalParams::Magic) + val,
                    cap),
                0);
            ServiceSystem::Get<ParameterService, ParameterMap>()
                ->set<WizardId::CATALYST>(CatalystParams::Magic, magic);
            mMagicText.tData.text = magic.toString() + "/" + cap.toString();
            mMagicText.renderText();
            break;
    }
}

void Catalyst::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    mMagicText.dest = Rect(mPos->rect.x(), mPos->rect.y2(), mPos->rect.w(), 0);
    mMagicText.dest.setHeight(FONT.h, Rect::Align::TOP_LEFT);
    mMagicText.fitToTexture();
    TextureBuilder().draw(mMagicText);
}

void Catalyst::calcMagicEffect() {
    auto params = ServiceSystem::Get<ParameterService, ParameterMap>();
    Number effect =
        (params->get<WizardId::CATALYST>(CatalystParams::Magic) + 1).logTen() +
        1;
    params->set<WizardId::CATALYST>(CatalystParams::MagicEffect, effect);
}
