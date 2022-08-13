#include "Wizard.h"

// Wizard
const std::string Wizard::POWER_UP_IMG = "res/upgrades/fireball_upgrade.png";
const std::string Wizard::SPEED_UP_IMG = "res/upgrades/speed_upgrade.png";

Wizard::Wizard() : WizardBase(WizardId::WIZARD) {}

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
    mWizUpdateSub =
        ServiceSystem::Get<WizardsDataService, WizardsDataObservable>()
            ->subscribe(std::bind(&Wizard::onWizardUpdate, this,
                                  std::placeholders::_1));

    // Target Upgrade
    Upgrade up{-1};
    up.setImg(WIZ_IMGS[mTarget]);
    up.setDescription("Change the Wizard's target");
    mTargetUp = mUpgrades->subscribe(
        [this](Upgrade& u) {
            u.mLevel = u.mLevel % 2;
            mTarget = u.mLevel == 0 ? WizardId::CRYSTAL : WizardId::CATALYST;
            u.mEffect = "Target: " + WIZ_NAMES[mTarget];
        },
        [this](Upgrade& u) { return 0; }, [this](Upgrade& u) { return true; },
        up);

    // Power Upgrade
    up = {1};
    up.setImg(POWER_UP_IMG);
    up.setDescription("Increase Wizard base power by 1");
    mPowerUp = mUpgrades->subscribe(
        [this](Upgrade& u) {
            mData->powerUp = u.mLevel;
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
            mData->speed = Number(1000) / timer.length;
            u.mEffect = mData->speed.toString() + "x";
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

void Wizard::onWizardUpdate(const WizardsData& params) {
    mPower = mBasePower;
    mPower += mData->powerUp;
    mPower *= params.crystal->magicEffect;
    mPower *= params.catalyst->magicEffect;
}

void Wizard::shootFireball() {
    mFireballs.push_back(std::move(ComponentFactory<Fireball>::New(
        mPos->rect.cX(), mPos->rect.cY(), mId, mTarget, mPower)));
}

const std::shared_ptr<WizardData>& Wizard::getData() const { return mData; }

// Crystal
Crystal::Crystal() : WizardBase(WizardId::CRYSTAL) {
    mMagicText.tData.font = AssetManager::getFont(FONT);
    mMagicText.tData.text = mData->magic.toString();
    mMagicText.renderText();
}

void Crystal::init() {
    WizardBase::init();
    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&Crystal::onRender, this, std::placeholders::_1), mPos);
    mTargetSub =
        ServiceSystem::Get<FireballService, TargetObservable>()->subscribe(
            std::bind(&Crystal::onHit, this, std::placeholders::_1,
                      std::placeholders::_2),
            mId);
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
            mData->magic += val;
            mMagicText.tData.text = mData->magic.toString();
            mMagicText.renderText();
            mData->magicEffect = mData->magic.logTenCopy() + 1;
            break;
    }
}

const std::shared_ptr<CrystalData>& Crystal::getData() const { return mData; }

// Catalyst
Catalyst::Catalyst() : WizardBase(WizardId::CATALYST) {
    mMagicText.tData.font = AssetManager::getFont(FONT);
    mMagicText.tData.text =
        mData->magic.toString() + "/" + mData->capacity.toString();
    mMagicText.renderText();
}

void Catalyst::init() {
    WizardBase::init();
    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&Catalyst::onRender, this, std::placeholders::_1), mPos);
    mTargetSub =
        ServiceSystem::Get<FireballService, TargetObservable>()->subscribe(
            std::bind(&Catalyst::onHit, this, std::placeholders::_1,
                      std::placeholders::_2),
            mId);
}

void Catalyst::onHit(WizardId src, Number val) {
    switch (src) {
        case WizardId::WIZARD:
            mData->magic = max(min(mData->magic + val, mData->capacity), 0);
            mData->magicEffect = mData->magic.logTenCopy() + 1;
            mMagicText.tData.text =
                mData->magic.toString() + "/" + mData->capacity.toString();
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

const std::shared_ptr<CatalystData>& Catalyst::getData() const { return mData; }
