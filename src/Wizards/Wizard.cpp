#include "Wizard.h"

// Wizard
const std::string Wizard::POWER_UP_IMG = "res/upgrades/fireball_upgrade.png";
const std::string Wizard::SPEED_UP_IMG = "res/upgrades/speed_upgrade.png";
const std::string Wizard::MULTI_UP_IMG = "res/upgrades/multi_upgrade.png";
const std::string Wizard::POWER_BKGRND = "res/wizards/power_effect_bkgrnd.png";
const std::string Wizard::FIREBALL_IMG = "res/projectiles/fireball.png";

Wizard::Wizard() : WizardBase(WIZARD) {
    auto params = ParameterSystem::Get();
    params->set<WIZARD>(WizardParams::BasePower, 1);
    params->set<WIZARD>(WizardParams::BaseSpeed, 1);
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
    mFireballTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            std::bind(&Wizard::onTimer, this), Timer(1000));
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

    auto params = ParameterSystem::Get();
    mParamSubs.push_back(
        params->subscribe<
            Keys<WIZARD, WizardParams::BasePower, WizardParams::PowerUp,
                 WizardParams::Speed, WizardParams::PowerWizEffect>,
            Keys<CRYSTAL, CrystalParams::MagicEffect>,
            Keys<CATALYST, CatalystParams::MagicEffect>>(
            std::bind(&Wizard::calcPower, this)));
    mParamSubs.push_back(
        params->subscribe<Keys<WIZARD, WizardParams::BaseSpeed>,
                          Keys<TIME_WIZARD, TimeWizardParams::SpeedEffect>>(
            std::bind(&Wizard::calcSpeed, this)));
    mParamSubs.push_back(params->subscribe<WIZARD>(
        WizardParams::Speed, std::bind(&Wizard::calcTimer, this)));
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
    float Multi =
        ParameterSystem::Get()->get<WIZARD>(WizardParams::MultiUp).toFloat();
    if (rDist(gen) < Multi) {
        shootFireball(SDL_FPoint{
            ((float)rDist(gen) - .5f) * mPos->rect.w() + mPos->rect.cX(),
            ((float)rDist(gen) - .5f) * mPos->rect.h() + mPos->rect.cY()});
    }
    return true;
}

void Wizard::onFireballHit(const Fireball& fireball) {
    auto params = ParameterSystem::Get();

    switch (fireball.getSourceId()) {
        case POWER_WIZARD: {
            params->set<WIZARD>(WizardParams::PowerWizEffect,
                                fireball.getValue(PowerWizardParams::Power));
            mPowWizTimerSub = TimeSystem::GetTimerObservable()->subscribe(
                [this]() {
                    ParameterSystem::Get()->set<WIZARD>(
                        WizardParams::PowerWizEffect, 1);
                    return false;
                },
                Timer(
                    fireball.getValue(PowerWizardParams::Duration).toFloat()));
        } break;
    }
}

void Wizard::onFireballFireRingHit(Fireball& fireball,
                                   const Number& fireRingEffect) {
    fireball.getValue() ^= fireRingEffect;
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
            if (mFireballFreezeCnt > 0) {
                FireballPtr& fireball = mFireballs.back();
                fireball->getValue() ^=
                    ParameterSystem::Get()->get<TIME_WIZARD>(
                        TimeWizardParams::FreezeEffect);
                std::cerr << "Do pretty graphics stuff" << std::endl;
            }
            break;
    }
}

void Wizard::shootFireball(SDL_FPoint launch) {
    bool frozen = TimeSystem::Frozen(TimeSystem::FreezeType::TIME_WIZARD);
    if (!frozen || mFireballFreezeCnt == 0) {
        mFireballs.push_back(std::move(ComponentFactory<Fireball>::New(
            SDL_FPoint{mPos->rect.cX(), mPos->rect.cY()}, mId, mTarget,
            FIREBALL_IMG,
            ParameterSystem::Get()->get<WIZARD>(WizardParams::Power))));
        if (!frozen) {
            mFireballs.back()->launch(launch);
        }
    }
    if (frozen) {
        if (mFireballFreezeCnt > 0) {
            mFireballs.back()->getValue() +=
                ParameterSystem::Get()->get<WIZARD>(WizardParams::Power);
        }
        mFireballFreezeCnt++;
        mFireballs.back()->setSize(
            fmin(pow(mFireballFreezeCnt, 1.0 / 3.0), 10));
    }
}

void Wizard::calcPower() {
    auto params = ParameterSystem::Get();
    Number power = (params->get<WIZARD>(WizardParams::BasePower) +
                    params->get<WIZARD>(WizardParams::PowerUp)) *
                   params->get<CRYSTAL>(CrystalParams::MagicEffect) *
                   params->get<CATALYST>(CatalystParams::MagicEffect) *
                   params->get<WIZARD>(WizardParams::PowerWizEffect) *
                   max(1, params->get<WIZARD>(WizardParams::Speed) * 16 / 1000);
    params->set<WIZARD>(WizardParams::Power, power);
}

void Wizard::calcSpeed() {
    auto params = ParameterSystem::Get();
    Number speed = params->get<WIZARD>(WizardParams::BaseSpeed) *
                   params->get<TIME_WIZARD>(TimeWizardParams::SpeedEffect);
    params->set<WIZARD>(WizardParams::Speed, speed);
}

void Wizard::calcTimer() {
    Timer& timer = mFireballTimerSub->get<TimerObservable::DATA>();
    float div =
        ParameterSystem::Get()->get<WIZARD>(WizardParams::Speed).toFloat();
    if (div <= 0) {
        div = 1;
    }
    timer.length = fmax(1000 / div, 16);
}

void Wizard::setPos(float x, float y) {
    WizardBase::setPos(x, y);
    if (TimeSystem::Frozen(TimeSystem::FreezeType::TIME_WIZARD) &&
        mFireballFreezeCnt > 0) {
        mFireballs.back()->setPos(mPos->rect.cX(), mPos->rect.cY());
    }
}
