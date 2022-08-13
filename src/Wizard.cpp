#include "Wizard.h"

// WizardBase
const Rect WizardBase::IMG_RECT(0, 0, 100, 100);
const FontData WizardBase::FONT{-1, IMG_RECT.H() / 4, "|"};

WizardBase::WizardBase(WizardId id)
    : mId(id),
      mPos(std::make_shared<UIComponent>(Rect(), Elevation::WIZARDS)),
      mDrag(std::make_shared<DragComponent>(250)) {}
WizardBase::~WizardBase() {}

void WizardBase::init() {
    setImage(WIZ_IMGS[mId]);
    SDL_Point screenDim = RenderSystem::getWindowSize();
    setPos(rDist(gen) * screenDim.x, rDist(gen) * screenDim.y);

    mResizeSub =
        ServiceSystem::Get<ResizeService, ResizeObservable>()->subscribe(
            std::bind(&WizardBase::onResize, this, std::placeholders::_1));
    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&WizardBase::onRender, this, std::placeholders::_1),
            mPos);
    mMouseSub = ServiceSystem::Get<MouseService, MouseObservable>()->subscribe(
        std::bind(&WizardBase::onClick, this, std::placeholders::_1,
                  std::placeholders::_2),
        mPos);
    mDragSub = ServiceSystem::Get<DragService, DragObservable>()->subscribe(
        []() {}, [this](int x, int y, float dx, float dy) { setPos(x, y); },
        []() {}, mPos, mDrag);
}

void WizardBase::onResize(ResizeData data) {
    setPos((float)mPos->rect.cX() * data.newW / data.oldW,
           (float)mPos->rect.cY() * data.newH / data.oldH);
}

void WizardBase::onRender(SDL_Renderer* r) {
    if (mDrag->dragging) {
        RectData rd;
        rd.color = GRAY;
        rd.set(mPos->rect, 5);
        TextureBuilder().draw(rd);
    }

    TextureBuilder().draw(mImg);
}

void WizardBase::onClick(Event::MouseButton b, bool clicked) {}

void WizardBase::setPos(float x, float y) {
    SDL_Point screenDim = RenderSystem::getWindowSize();
    mPos->rect.setPos(x, y, Rect::Align::CENTER);
    mPos->rect.fitWithin(Rect(0, 0, screenDim.x, screenDim.y));
    mImg.dest = mPos->rect;
    ServiceSystem::Get<FireballService, FireballObservable>()->next(
        mId, {mPos->rect.cX(), mPos->rect.cY()});
}

void WizardBase::setImage(const std::string& img) {
    mImg.texture = AssetManager::getTexture(img);
    mImg.dest = IMG_RECT;
    mImg.dest.setPos(mPos->rect.cX(), mPos->rect.cY(), Rect::Align::CENTER);
    mImg.fitToTexture();
    mPos->rect = mImg.dest;
}

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
    mWizUpdateSub = WizardParameters::Subscribe<WizardParams>(
        std::bind(&Wizard::onWizardUpdate, this, std::placeholders::_1));

    // Target Upgrade
    Upgrade up{-1};
    up.setImg(WIZ_IMGS[mTarget]);
    up.setDescription("Change the Wizard's target");
    mTargetUp = mUpgrades->subscribe(
        [this](Upgrade& u) {
            mTarget = mTarget == WizardId::CRYSTAL ? WizardId::CATALYST
                                                   : WizardId::CRYSTAL;
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
            WizardParameters::Set(WizardParams::WizardPowerUpgrade, u.mLevel);
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
            u.mEffect = (Number(1000) / timer.length).toString() + "x";
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

void Wizard::onWizardUpdate(const ParameterList<WizardParams>& params) {
    mPower = mBasePower;
    mPower += WizardParameters::Get(WizardParams::WizardPowerUpgrade, 0);
    mPower *= WizardParameters::Get(WizardParams::CrystalMagic, 1);
    mPower *= WizardParameters::Get(WizardParams::CatalystMagic, 1);
}

void Wizard::shootFireball() {
    mFireballs.push_back(std::move(ComponentFactory<Fireball>::New(
        mPos->rect.cX(), mPos->rect.cY(), mId, mTarget, mPower)));
}

// Crystal
Crystal::Crystal() : WizardBase(WizardId::CRYSTAL) {
    mMagicText.tData.font = AssetManager::getFont(FONT);
    mMagicText.tData.text = mMagic.toString();
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
            mMagic += val;
            mMagicText.tData.text = mMagic.toString();
            mMagicText.renderText();
            WizardParameters::Set(WizardParams::CrystalMagic,
                                  mMagic.logTenCopy() + 1);
            break;
    }
}

// Catalyst
Catalyst::Catalyst() : WizardBase(WizardId::CATALYST) {
    mMagicText.tData.font = AssetManager::getFont(FONT);
    mMagicText.tData.text = mMagic.toString() + "/" + mCapacity.toString();
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
            mMagic = max(min(mMagic + val, mCapacity), 0);
            mMagicText.tData.text =
                mMagic.toString() + "/" + mCapacity.toString();
            mMagicText.renderText();
            WizardParameters::Set(WizardParams::CatalystMagic,
                                  mMagic.logTenCopy() + 1);
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
