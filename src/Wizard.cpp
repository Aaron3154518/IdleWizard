#include "Wizard.h"

// WizardBase
const Rect WizardBase::IMG_RECT(0, 0, 100, 100);
const FontData WizardBase::FONT{-1, IMG_RECT.H() / 4, "|"};

WizardBase::WizardBase(WizardId id)
    : mId(id),
      mComp(std::make_shared<DragComponent>(Rect(), Elevation::WIZARDS, 250)) {}
WizardBase::~WizardBase() {}

void WizardBase::init() {
    setImage(WIZ_IMGS[mId]);
    SDL_Point screenDim = RenderSystem::getWindowSize();
    setPos(rDist(gen) * screenDim.x, rDist(gen) * screenDim.y);

    mComp->onDrag = [this](int x, int y, float dx, float dy) { setPos(x, y); };
    mComp->onDragStart = []() {};
    mComp->onDragEnd = []() {};

    mResizeSub =
        ServiceSystem::Get<ResizeService, ResizeObservable>()->subscribe(
            std::bind(&WizardBase::onResize, this, std::placeholders::_1));
    mResizeSub->setUnsubscriber(unsub);
    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&WizardBase::onRender, this, std::placeholders::_1),
            mComp);
    mRenderSub->setUnsubscriber(unsub);
    mMouseSub = ServiceSystem::Get<MouseService, MouseObservable>()->subscribe(
        std::bind(&WizardBase::onClick, this, std::placeholders::_1,
                  std::placeholders::_2),
        mComp);
    mMouseSub->setUnsubscriber(unsub);
    mDragSub =
        ServiceSystem::Get<DragService, DragObservable>()->subscribe(mComp);
    mDragSub->setUnsubscriber(unsub);
}

void WizardBase::onResize(ResizeData data) {
    setPos((float)mComp->rect.cX() * data.newW / data.oldW,
           (float)mComp->rect.cY() * data.newH / data.oldH);
}

void WizardBase::onRender(SDL_Renderer* r) {
    if (mComp->dragging) {
        RectData rd;
        rd.color = GRAY;
        rd.set(mComp->rect, 5);
        TextureBuilder().draw(rd);
    }

    TextureBuilder().draw(mImg);
}

void WizardBase::onClick(Event::MouseButton b, bool clicked) {}

void WizardBase::setPos(float x, float y) {
    SDL_Point screenDim = RenderSystem::getWindowSize();
    mComp->rect.setPos(x, y, Rect::Align::CENTER);
    mComp->rect.fitWithin(Rect(0, 0, screenDim.x, screenDim.y));
    mImg.dest = mComp->rect;
    ServiceSystem::Get<FireballService, FireballObservable>()->next(
        mId, {mComp->rect.cX(), mComp->rect.cY()});
}

void WizardBase::setImage(const std::string& img) {
    mImg.texture = AssetManager::getTexture(img);
    mImg.dest = IMG_RECT;
    mImg.dest.setPos(mComp->rect.cX(), mComp->rect.cY(), Rect::Align::CENTER);
    mImg.fitToTexture();
    mComp->rect = mImg.dest;
}

// Wizard
const std::string Wizard::POWER_UP_IMG = "res/upgrades/fireball_upgrade.png";
const std::string Wizard::SPEED_UP_IMG = "res/upgrades/speed_upgrade.png";

Wizard::Wizard() : WizardBase(WizardId::WIZARD) {
    std::shared_ptr<Upgrade> upgrade;

    // Target Upgrade
    upgrade = std::make_shared<Upgrade>();
    upgrade->onClick = [this]() {
        mTarget = mTarget == WizardId::CRYSTAL ? WizardId::CATALYST
                                               : WizardId::CRYSTAL;
    };
    upgrade->status = [this]() { return Upgrade::Status::CAN_BUY; };
    upgrade->getImage = [this]() {
        return AssetManager::getTexture(WIZ_IMGS[mTarget]);
    };
    mUpgrades.push_back(upgrade);

    // Power Upgrade
    upgrade = std::make_shared<Upgrade>();
    upgrade->onClick = [this]() {
        ServiceSystem::Get<WizardUpdateService, WizardParameters>()->setParam(
            WizardParams::WizardPowerUpgrade, 1);
        mPowerBought = true;
    };
    upgrade->status = [this]() {
        return mPowerBought ? Upgrade::Status::BOUGHT
                            : Upgrade::Status::CAN_BUY;
    };
    upgrade->getImage = [this]() {
        return AssetManager::getTexture(POWER_UP_IMG);
    };
    mUpgrades.push_back(upgrade);

    // Speed Upgrade
    upgrade = std::make_shared<Upgrade>();
    upgrade->onClick = [this]() {
        mSpeedBoughtCnt++;
        mTimerSub->getData()->length = 1000 * pow(.75, mSpeedBoughtCnt);
    };
    upgrade->status = [this]() {
        return mSpeedBoughtCnt < 5 ? Upgrade::Status::CAN_BUY
                                   : Upgrade::Status::BOUGHT;
    };
    upgrade->getImage = [this]() {
        return AssetManager::getTexture(SPEED_UP_IMG);
    };
    mUpgrades.push_back(upgrade);
}

void Wizard::init() {
    WizardBase::init();

    ServiceSystem::Get<RenderService, RenderObservable>()->updateSubscription(
        mRenderSub, std::bind(&Wizard::onRender, this, std::placeholders::_1));
    ServiceSystem::Get<MouseService, MouseObservable>()->updateSubscription(
        mMouseSub, std::bind(&Wizard::onClick, this, std::placeholders::_1,
                             std::placeholders::_2));

    mTimerSub = ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
        std::bind(&Wizard::onTimer, this), 1000);
    mTimerSub->setUnsubscriber(unsub);
    mWizUpdateSub =
        ServiceSystem::Get<WizardUpdateService, WizardParameters>()->subscribe(
            std::bind(&Wizard::onWizardUpdate, this, std::placeholders::_1));
    mWizUpdateSub->setUnsubscriber(unsub);
}

void Wizard::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    for (auto it = mFireballs.begin(); it != mFireballs.end(); ++it) {
        if ((*it)->dead()) {
            std::cerr << "Erase: " << it->get() << std::endl;
            it = mFireballs.erase(it);
            std::cerr << "Done" << std::endl;
            if (it == mFireballs.end()) {
                break;
            }
        }
    }
}

void Wizard::onClick(Event::MouseButton b, bool clicked) {
    if (clicked) {
        ServiceSystem::Get<UpgradeService, UpgradeObservable>()->next(
            mUpgrades);
    }
}

bool Wizard::onTimer() {
    shootFireball();
    return true;
}

void Wizard::onWizardUpdate(const ParameterList<WizardParams>& params) {
    auto wizUpdate =
        ServiceSystem::Get<WizardUpdateService, WizardParameters>();

    mPower = mBasePower;
    mPower += wizUpdate->getParam(WizardParams::WizardPowerUpgrade, 0);
    mPower *= wizUpdate->getParam(WizardParams::CrystalMagic, 1);
    mPower *= wizUpdate->getParam(WizardParams::CatalystMagic, 1);
}

void Wizard::shootFireball() {
    mFireballs.push_back(std::move(ComponentFactory<Fireball>::New(
        mComp->rect.cX(), mComp->rect.cY(), mId, mTarget, mPower)));
    std::cerr << "Shoot: " << mFireballs.back().get() << std::endl;
}

// Crystal
Crystal::Crystal() : WizardBase(WizardId::CRYSTAL) {
    mMagicText.tData.font = AssetManager::getFont(FONT);
    mMagicText.tData.text = mMagic.toString();
    mMagicText.renderText();
}

void Crystal::init() {
    WizardBase::init();
    ServiceSystem::Get<RenderService, RenderObservable>()->updateSubscription(
        mRenderSub, std::bind(&Crystal::onRender, this, std::placeholders::_1));
    mTargetSub =
        ServiceSystem::Get<FireballService, TargetObservable>()->subscribe(
            std::bind(&Crystal::onHit, this, std::placeholders::_1,
                      std::placeholders::_2),
            std::make_shared<WizardId>(mId));
    mTargetSub->setUnsubscriber(unsub);
}

void Crystal::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    mMagicText.dest =
        Rect(mComp->rect.x(), mComp->rect.y(), mComp->rect.w(), 0);
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
            ServiceSystem::Get<WizardUpdateService, WizardParameters>()
                ->setParam(WizardParams::CrystalMagic, mMagic.logTenCopy() + 1);
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
    ServiceSystem::Get<RenderService, RenderObservable>()->updateSubscription(
        mRenderSub,
        std::bind(&Catalyst::onRender, this, std::placeholders::_1));
    mTargetSub =
        ServiceSystem::Get<FireballService, TargetObservable>()->subscribe(
            std::bind(&Catalyst::onHit, this, std::placeholders::_1,
                      std::placeholders::_2),
            std::make_shared<WizardId>(mId));
    mTargetSub->setUnsubscriber(unsub);
}

void Catalyst::onHit(WizardId src, Number val) {
    switch (src) {
        case WizardId::WIZARD:
            mMagic = max(min(mMagic + val, mCapacity), 0);
            mMagicText.tData.text =
                mMagic.toString() + "/" + mCapacity.toString();
            mMagicText.renderText();
            ServiceSystem::Get<WizardUpdateService, WizardParameters>()
                ->setParam(WizardParams::CatalystMagic,
                           mMagic.logTenCopy() + 1);
            break;
    }
}

void Catalyst::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    mMagicText.dest =
        Rect(mComp->rect.x(), mComp->rect.y2(), mComp->rect.w(), 0);
    mMagicText.dest.setHeight(FONT.h, Rect::Align::TOP_LEFT);
    mMagicText.fitToTexture();
    TextureBuilder().draw(mMagicText);
}
