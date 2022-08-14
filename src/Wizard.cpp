#include "Wizard.h"

// Wizard
const std::string Wizard::POWER_UP_IMG = "res/upgrades/fireball_upgrade.png";
const std::string Wizard::SPEED_UP_IMG = "res/upgrades/speed_upgrade.png";

Wizard::Wizard() : WizardBase(WIZARD) {
    auto params = Parameters();
    params->set<WIZARD>(WizardParams::PowerUpgrade, 0);
    params->set<WIZARD>(WizardParams::Speed, 1);
}

void Wizard::init() {
    WizardBase::init();

    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&Wizard::onRender, this, std::placeholders::_1), mPos);
    mTimerSub = ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
        std::bind(&Wizard::onTimer, this), Timer(1000));

    // Power Display
    Upgrade up{0};
    up.setImg(WIZ_IMGS.at(mId));
    up.setDescription("Current power");
    mPowerDisplay =
        mUpgrades->subscribe([](Upgrade& u) {}, [](Upgrade& u) { return 0; },
                             [](Upgrade& u) { return true; }, up);

    // Target Upgrade
    up = {-1};
    up.setImg(WIZ_IMGS.at(mTarget));
    up.setDescription("Change the Wizard's target");
    mTargetUp = mUpgrades->subscribe(
        [this](Upgrade& u) {
            u.mLevel = u.mLevel % 2;
            mTarget = u.mLevel == 0 ? CRYSTAL : CATALYST;
            u.mEffect = "Target: " + WIZ_NAMES.at(mTarget);
            u.setImg(WIZ_IMGS.at(mTarget));
        },
        [this](Upgrade& u) { return 0; }, [this](Upgrade& u) { return true; },
        up);

    // Power Upgrade
    up = {1};
    up.setImg(POWER_UP_IMG);
    up.setDescription("Increase Wizard base power by 1");
    mPowerUp = mUpgrades->subscribe(
        [this](Upgrade& u) {
            Parameters()->set<WIZARD>(WizardParams::PowerUpgrade, u.mLevel);
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
            Number speed = (Number(4. / 3.) ^ u.mLevel);
            Parameters()->set<WIZARD>(WizardParams::Speed, speed);
            u.mEffect = speed.toString() + "x";
        },
        [this](Upgrade& u) { return (Number(1.5) ^ u.mLevel) * 100; },
        [this](Upgrade& u) { return true; }, up);

    auto params = Parameters();
    mParamSubs.push_back(
        params->subscribe<
            Keys<WIZARD, WizardParams::PowerUpgrade, WizardParams::Speed>,
            Keys<CRYSTAL, CrystalParams::MagicEffect>,
            Keys<CATALYST, CatalystParams::MagicEffect>>(
            std::bind(&Wizard::calcPower, this)));
    mParamSubs.push_back(
        params->subscribe<WIZARD>(WizardParams::Speed, [this]() {
            Timer& timer = mTimerSub->get<TimerObservable::DATA>();
            timer.length = fmax(
                1000 / Parameters()->get<WIZARD>(WizardParams::Speed).tofloat(),
                16);
        }));
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

bool Wizard::onTimer() {
    shootFireball();
    return true;
}

void Wizard::shootFireball() {
    mFireballs.push_back(std::move(ComponentFactory<Fireball>::New(
        mPos->rect.cX(), mPos->rect.cY(), mId, mTarget,
        Parameters()->get<WIZARD>(WizardParams::Power))));
}

void Wizard::calcPower() {
    auto params = Parameters();
    Number power = (1 + params->get<WIZARD>(WizardParams::PowerUpgrade)) *
                   params->get<CRYSTAL>(CrystalParams::MagicEffect) *
                   params->get<CATALYST>(CatalystParams::MagicEffect) *
                   max(1, params->get<WIZARD>(WizardParams::Speed) * 16 / 1000);
    params->set<WIZARD>(WizardParams::Power, power);
    if (mPowerDisplay) {
        Upgrade& up = Upgrade::Get(mPowerDisplay);
        up.mEffect = power.toString() + "x";
        up.updateInfo();
    }
}

// Crystal
Crystal::Crystal() : WizardBase(CRYSTAL) {
    auto params = Parameters();
    params->set<CRYSTAL>(CrystalParams::Magic, 0);
}

void Crystal::init() {
    WizardBase::init();

    mMagicText.tData.font = AssetManager::getFont(FONT);

    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&Crystal::onRender, this, std::placeholders::_1), mPos);
    mTargetSub =
        ServiceSystem::Get<FireballService, TargetObservable>()->subscribe(
            std::bind(&Crystal::onHit, this, std::placeholders::_1,
                      std::placeholders::_2),
            mId);

    // Power Display
    Upgrade up{0};
    up.setImg(WIZ_IMGS.at(mId));
    up.setDescription("Multiplier based on crystal damage");
    mMagicEffectDisplay =
        mUpgrades->subscribe([](Upgrade& u) {}, [](Upgrade& u) { return 0; },
                             [](Upgrade& u) { return true; }, up);

    auto params = Parameters();
    mParamSubs.push_back(params->subscribe<CRYSTAL>(
        CrystalParams::Magic, std::bind(&Crystal::calcMagicEffect, this)));
    mParamSubs.push_back(params->subscribe<CRYSTAL>(
        CrystalParams::Magic, std::bind(&Crystal::drawMagic, this)));
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
        case WIZARD:
            auto params = Parameters();
            Number magic = params->get<CRYSTAL>(CrystalParams::Magic) + val;
            params->set<CRYSTAL>(CrystalParams::Magic, magic);
            mMagicText.tData.text = magic.toString();
            mMagicText.renderText();
            break;
    }
}

void Crystal::calcMagicEffect() {
    auto params = Parameters();
    Number effect =
        (params->get<CRYSTAL>(CrystalParams::Magic) + 1).logTen() + 1;
    params->set<CRYSTAL>(CrystalParams::MagicEffect, effect);
    if (mMagicEffectDisplay) {
        Upgrade& up = Upgrade::Get(mMagicEffectDisplay);
        up.mEffect = effect.toString() + "x";
        up.updateInfo();
    }
}

void Crystal::drawMagic() {
    mMagicText.tData.text =
        Parameters()->get<CRYSTAL>(CrystalParams::Magic).toString();
    mMagicText.renderText();
}

// Catalyst
Catalyst::Catalyst() : WizardBase(CATALYST) {
    auto params = Parameters();
    params->set<CATALYST>(CatalystParams::Magic, 0);
    params->set<CATALYST>(CatalystParams::Capacity, 100);
}

void Catalyst::init() {
    WizardBase::init();

    mMagicText.tData.font = AssetManager::getFont(FONT);

    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&Catalyst::onRender, this, std::placeholders::_1), mPos);
    mTargetSub =
        ServiceSystem::Get<FireballService, TargetObservable>()->subscribe(
            std::bind(&Catalyst::onHit, this, std::placeholders::_1,
                      std::placeholders::_2),
            mId);

    // Power Display
    Upgrade up{0};
    up.setImg(WIZ_IMGS.at(mId));
    up.setDescription("Multiplier from stored magic");
    mMagicEffectDisplay =
        mUpgrades->subscribe([](Upgrade& u) {}, [](Upgrade& u) { return 0; },
                             [](Upgrade& u) { return true; }, up);

    auto params = Parameters();
    mParamSubs.push_back(params->subscribe<CATALYST>(
        CatalystParams::Magic, std::bind(&Catalyst::calcMagicEffect, this)));
    mParamSubs.push_back(
        params->subscribe<
            Keys<CATALYST, CatalystParams::Magic, CatalystParams::Capacity>>(
            std::bind(&Catalyst::drawMagic, this)));
}

void Catalyst::onHit(WizardId src, Number val) {
    switch (src) {
        case WIZARD:
            auto params = Parameters();
            Number magic =
                max(min(params->get<CATALYST>(CatalystParams::Magic) + val,
                        params->get<CATALYST>(CatalystParams::Capacity)),
                    0);
            params->set<CATALYST>(CatalystParams::Magic, magic);
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
    auto params = Parameters();
    Number effect =
        (params->get<CATALYST>(CatalystParams::Magic) + 1).logTen() + 1;
    params->set<CATALYST>(CatalystParams::MagicEffect, effect);
    if (mMagicEffectDisplay) {
        Upgrade& up = Upgrade::Get(mMagicEffectDisplay);
        up.mEffect = effect.toString() + "x";
        up.updateInfo();
    }
}

void Catalyst::drawMagic() {
    mMagicText.tData.text =
        Parameters()->get<CATALYST>(CatalystParams::Magic).toString() + "/" +
        Parameters()->get<CATALYST>(CatalystParams::Capacity).toString();
    mMagicText.renderText();
}
