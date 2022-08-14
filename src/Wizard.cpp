#include "Wizard.h"

// Wizard
const std::string Wizard::POWER_UP_IMG = "res/upgrades/fireball_upgrade.png";
const std::string Wizard::SPEED_UP_IMG = "res/upgrades/speed_upgrade.png";
const std::string Wizard::MULTI_UP_IMG = "res/upgrades/multi_upgrade.png";
const std::string Wizard::POWER_BKGRND = "res/wizards/power_effect_bkgrnd.png";
const std::string Wizard::FIREBALL_IMG = "res/projectiles/fireball.png";

Wizard::Wizard() : WizardBase(WIZARD) {
    auto params = Parameters();
    params->set<WIZARD>(WizardParams::Power, 1);
    params->set<WIZARD>(WizardParams::Speed, 1);
    params->set<WIZARD>(WizardParams::PowerUp, 0);
    params->set<WIZARD>(WizardParams::MultiUp, 0);
    params->set<WIZARD>(WizardParams::PowerWizEffect, 1);
}

void Wizard::init() {
    WizardBase::init();

    mPowBkgrnd.texture = AssetManager::getTexture(POWER_BKGRND);

    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&Wizard::onRender, this, std::placeholders::_1), mPos);
    mTimerSub = ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
        std::bind(&Wizard::onTimer, this), Timer(1000));
    mTargetSub =
        ServiceSystem::Get<FireballService, TargetObservable>()->subscribe(
            std::bind(&Wizard::onHit, this, std::placeholders::_1,
                      std::placeholders::_2),
            mId);

    // Power Display
    UpgradePtr up = std::make_shared<Upgrade>();
    up->setMaxLevel(0)
        .setEffectSource<WIZARD, WizardParams::Power>(
            Upgrade::Defaults::MultiplicativeEffect)
        .setImg(WIZ_IMGS.at(mId))
        .setDescription("Power");
    mPowerDisplay = mUpgrades->subscribe(up);

    // Target Upgrade
    up = std::make_shared<Upgrade>();
    up->setMaxLevel(-1)
        .setImg(WIZ_IMGS.at(mTarget))
        .setDescription("Change the Wizard's target");
    mTargetUp = mUpgrades->subscribe(
        [this](UpgradePtr u) {
            mTarget = u->getLevel() % 2 == 0 ? CRYSTAL : CATALYST;
            u->setLevel(u->getLevel() % 2)
                .setEffect("Target: " + WIZ_NAMES.at(mTarget))
                .setImg(WIZ_IMGS.at(mTarget));
        },
        up);

    // Power Upgrade
    up = std::make_shared<Upgrade>();
    up->setMaxLevel(5)
        .setCostSource<WIZARD, WizardParams::PowerUpCost>()
        .setMoneySource(Upgrade::ParamSources::CRYSTAL_MAGIC)
        .setEffectSource<WIZARD, WizardParams::PowerUp>(
            Upgrade::Defaults::AdditiveEffect)
        .setImg(POWER_UP_IMG)
        .setDescription("Increase Wizard base power by 1");
    mPowerUp = mUpgrades->subscribe(
        [this](UpgradePtr u) {
            u->getEffectSrc().set(u->getLevel());
            u->getCostSrc().set(25 * (Number(1.75) ^ u->getLevel()));
        },
        up);

    // Speed Upgrade
    up = std::make_shared<Upgrade>();
    up->setMaxLevel(20)
        .setCostSource<WIZARD, WizardParams::MultiUpCost>()
        .setMoneySource(Upgrade::ParamSources::CRYSTAL_MAGIC)
        .setEffectSource<WIZARD, WizardParams::MultiUp>(
            Upgrade::Defaults::PercentEffect)
        .setImg(MULTI_UP_IMG)
        .setDescription("Increase double fireball chance by +5%");
    mMultiUp = mUpgrades->subscribe(
        [this](UpgradePtr u) {
            u->getEffectSrc().set(Number(.05) * u->getLevel());
            u->getCostSrc().set((Number(1.5) ^ u->getLevel()) * 100);
        },
        up);

    auto params = Parameters();
    mParamSubs.push_back(
        params
            ->subscribe<Keys<WIZARD, WizardParams::PowerUp, WizardParams::Speed,
                             WizardParams::PowerWizEffect>,
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
    if (mPowWizTimerSub &&
        mPowWizTimerSub->get<TimerObservable::DATA>().isActive()) {
        mPowBkgrnd.dest = mPos->rect;
        mPowBkgrnd.fitToTexture();
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

bool Wizard::onTimer() {
    shootFireball();
    float Multi = Parameters()->get<WIZARD>(WizardParams::MultiUp).tofloat();
    if (rDist(gen) < Multi) {
        shootFireball()->launch(
            {((float)rDist(gen) - .5f) * mPos->rect.w() + mPos->rect.cX(),
             ((float)rDist(gen) - .5f) * mPos->rect.h() + mPos->rect.cY()});
    }
    return true;
}

void Wizard::onHit(WizardId src, const Number& power) {
    auto params = Parameters();

    switch (src) {
        case POWER_WIZARD: {
            params->set<WIZARD>(
                WizardParams::PowerWizEffect,
                params->get<POWER_WIZARD>(PowerWizardParams::Power));
            mPowWizTimerSub =
                ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
                    [this]() {
                        Parameters()->set<WIZARD>(WizardParams::PowerWizEffect,
                                                  1);
                        return false;
                    },
                    Timer(params->get<POWER_WIZARD>(PowerWizardParams::Duration)
                              .tofloat()));
        } break;
    }
}

std::unique_ptr<Fireball>& Wizard::shootFireball() {
    mFireballs.push_back(std::move(ComponentFactory<Fireball>::New(
        SDL_FPoint{mPos->rect.cX(), mPos->rect.cY()}, mId, mTarget,
        Parameters()->get<WIZARD>(WizardParams::Power), FIREBALL_IMG)));
    return mFireballs.back();
}

void Wizard::calcPower() {
    auto params = Parameters();
    Number power = (1 + params->get<WIZARD>(WizardParams::PowerUp)) *
                   params->get<CRYSTAL>(CrystalParams::MagicEffect) *
                   params->get<CATALYST>(CatalystParams::MagicEffect) *
                   params->get<WIZARD>(WizardParams::PowerWizEffect) *
                   max(1, params->get<WIZARD>(WizardParams::Speed) * 16 / 1000);
    params->set<WIZARD>(WizardParams::Power, power);
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
    mMouseSub = ServiceSystem::Get<MouseService, MouseObservable>()->subscribe(
        std::bind(&Crystal::onClick, this, std::placeholders::_1,
                  std::placeholders::_2),
        mPos);
    mTargetSub =
        ServiceSystem::Get<FireballService, TargetObservable>()->subscribe(
            std::bind(&Crystal::onHit, this, std::placeholders::_1,
                      std::placeholders::_2),
            mId);

    // Power Display
    UpgradePtr up = std::make_shared<Upgrade>();
    up->setMaxLevel(0)
        .setEffectSource<CRYSTAL, CrystalParams::MagicEffect>(
            Upgrade::Defaults::MultiplicativeEffect)
        .setImg(WIZ_IMGS.at(mId))
        .setDescription("Multiplier based on crystal damage");
    mMagicEffectDisplay = mUpgrades->subscribe(up);

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

    for (auto it = mFireRings.begin(); it != mFireRings.end(); ++it) {
        if ((*it)->dead()) {
            it = mFireRings.erase(it);
            if (it == mFireRings.end()) {
                break;
            }
        }
    }
}

void Crystal::onClick(Event::MouseButton b, bool clicked) {
    WizardBase::onClick(b, clicked);
    if (clicked) {
        auto params = Parameters();
        params->set<CRYSTAL>(CrystalParams::Magic,
                             params->get<CRYSTAL>(CrystalParams::Magic) * 10);
    }
}

void Crystal::onHit(WizardId src, const Number& val) {
    auto params = Parameters();

    switch (src) {
        case WIZARD: {
            Number magic = params->get<CRYSTAL>(CrystalParams::Magic) + val;
            params->set<CRYSTAL>(CrystalParams::Magic, magic);
            mMagicText.tData.text = magic.toString();
            mMagicText.renderText();
        } break;
        case POWER_WIZARD: {
            createFireRing();
        } break;
    }
}

void Crystal::calcMagicEffect() {
    auto params = Parameters();
    Number effect =
        (params->get<CRYSTAL>(CrystalParams::Magic) + 1).logTen() + 1;
    params->set<CRYSTAL>(CrystalParams::MagicEffect, effect);
}

void Crystal::drawMagic() {
    mMagicText.tData.text =
        Parameters()->get<CRYSTAL>(CrystalParams::Magic).toString();
    mMagicText.renderText();
}

std::unique_ptr<FireRing>& Crystal::createFireRing() {
    mFireRings.push_back(std::move(ComponentFactory<FireRing>::New(
        SDL_Point{mPos->rect.CX(), mPos->rect.CY()},
        Parameters()->get<POWER_WIZARD>(PowerWizardParams::FireRingEffect))));
    return mFireRings.back();
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
    UpgradePtr up = std::make_shared<Upgrade>();
    up->setMaxLevel(0)
        .setEffectSource<CATALYST, CatalystParams::MagicEffect>(
            Upgrade::Defaults::MultiplicativeEffect)
        .setImg(WIZ_IMGS.at(mId))
        .setDescription("Multiplier from stored magic");
    mMagicEffectDisplay = mUpgrades->subscribe(up);

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
}

void Catalyst::drawMagic() {
    mMagicText.tData.text =
        Parameters()->get<CATALYST>(CatalystParams::Magic).toString() + "/" +
        Parameters()->get<CATALYST>(CatalystParams::Capacity).toString();
    mMagicText.renderText();
}

// PowerWizard
const std::string PowerWizard::FIREBALL_IMG = "res/projectiles/fireball2.png";

PowerWizard::PowerWizard() : WizardBase(POWER_WIZARD) {
    auto params = Parameters();
    params->set<POWER_WIZARD>(PowerWizardParams::Power, 5);
    params->set<POWER_WIZARD>(PowerWizardParams::FireRingEffect, 1);
    params->set<POWER_WIZARD>(PowerWizardParams::Speed, .25);
    params->set<POWER_WIZARD>(PowerWizardParams::Duration, 1000);
}

void PowerWizard::init() {
    WizardBase::init();

    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&PowerWizard::onRender, this, std::placeholders::_1),
            mPos);
    mTimerSub = ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
        std::bind(&PowerWizard::onTimer, this), Timer(1000));

    // Power Display
    UpgradePtr up = std::make_shared<Upgrade>();
    up->setMaxLevel(0)
        .setEffectSource<POWER_WIZARD, PowerWizardParams::Power>(
            Upgrade::Defaults::MultiplicativeEffect)
        .setImg(WIZ_IMGS.at(mId))
        .setDescription("Power");
    mPowerDisplay = mUpgrades->subscribe(up);

    auto params = Parameters();
    mParamSubs.push_back(params->subscribe<POWER_WIZARD>(
        PowerWizardParams::Power,
        std::bind(&PowerWizard::calcFireRingEffect, this)));
    mParamSubs.push_back(
        params->subscribe<POWER_WIZARD>(PowerWizardParams::Speed, [this]() {
            Timer& timer = mTimerSub->get<TimerObservable::DATA>();
            timer.length =
                fmax(1000 / Parameters()
                                ->get<POWER_WIZARD>(PowerWizardParams::Speed)
                                .tofloat(),
                     16);
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

bool PowerWizard::onTimer() {
    shootFireball();
    return true;
}

std::unique_ptr<Fireball>& PowerWizard::shootFireball() {
    WizardId target = rDist(gen) < .5 ? WIZARD : CRYSTAL;
    mFireballs.push_back(std::move(ComponentFactory<Fireball>::New(
        SDL_FPoint{mPos->rect.cX(), mPos->rect.cY()}, mId, target,
        Parameters()->get<POWER_WIZARD>(PowerWizardParams::Power),
        FIREBALL_IMG)));
    return mFireballs.back();
}

void PowerWizard::calcFireRingEffect() {
    auto params = Parameters();
    Number effect =
        (params->get<POWER_WIZARD>(PowerWizardParams::Power).logTenCopy() + 1)
            .sqrt();
    params->set<POWER_WIZARD>(PowerWizardParams::FireRingEffect, effect);
}
