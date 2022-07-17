#include "Wizard.h"

// WizardBase
const Rect WizardBase::BORDER_RECT(0, 0, 500, 500);
const Rect WizardBase::IMG_RECT(0, 0, 100, 100);
const FontData WizardBase::FONT{-1, IMG_RECT.H() / 4, "|"};

WizardBase::WizardBase(WizardId id)
    : mId(id), mComp(std::make_shared<DragComponent>(Rect(), 1, 250)) {}
WizardBase::~WizardBase() {}

void WizardBase::init() {
    mBorder.set(BORDER_RECT, 2, true);

    setImage(WIZ_IMGS[mId]);
    setPos(rDist(gen) * BORDER_RECT.w(), rDist(gen) * BORDER_RECT.h());

    mComp->onDrag = [this](int x, int y, float dx, float dy) { setPos(x, y); };
    mComp->onDragStart = []() {};
    mComp->onDragEnd = []() {};

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

void WizardBase::onRender(SDL_Renderer* r) {
    if (mComp->dragging) {
        RectData rd;
        rd.color = GRAY;
        rd.set(mComp->rect, 5);
        TextureBuilder().draw(rd);
    }

    TextureBuilder().draw(mImg);

    TextureBuilder().draw(mBorder);
}

void WizardBase::onClick(Event::MouseButton b, bool clicked) {}

void WizardBase::setPos(float x, float y) {
    mComp->rect.setPos(x, y, Rect::Align::CENTER);
    mComp->rect.fitWithin(BORDER_RECT);
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
Wizard::Wizard() : WizardBase(WizardId::WIZARD) {}

void Wizard::init() {
    WizardBase::init();

    ServiceSystem::Get<RenderService, RenderObservable>()->updateSubscription(
        mRenderSub, std::bind(&Wizard::onRender, this, std::placeholders::_1));
    ServiceSystem::Get<MouseService, MouseObservable>()->updateSubscription(
        mMouseSub, std::bind(&Wizard::onClick, this, std::placeholders::_1,
                             std::placeholders::_2));

    mWizUpdateSub =
        ServiceSystem::Get<WizardUpdateService, WizardParameters>()->subscribe(
            std::bind(&Wizard::onWizardUpdate, this, std::placeholders::_1));
    mWizUpdateSub->setUnsubscriber(unsub);
}

void Wizard::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    for (auto it = mFireballs.begin(); it != mFireballs.end(); ++it) {
        if (!(*it)->dead()) {
            it = mFireballs.erase(it);
            if (it == mFireballs.end()) {
                break;
            }
        }
    }
}

void Wizard::onClick(Event::MouseButton b, bool clicked) {
    if (clicked) {
        shootFireball(WizardId::CRYSTAL);
    }
}

void Wizard::onWizardUpdate(const ParameterList<WizardParams>& params) {
    auto wizUpdate =
        ServiceSystem::Get<WizardUpdateService, WizardParameters>();

    mPower = mBasePower;
    mPower *= wizUpdate->getParam(WizardParams::CrystalMagic, Number(1));
}

void Wizard::shootFireball(WizardId target) {
    mFireballs.push_back(std::move(ComponentFactory<Fireball>::New(
        mComp->rect.cX(), mComp->rect.cY(), mId, target, mPower)));
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
Catalyst::Catalyst() : WizardBase(WizardId::CATALYST) {}

void Catalyst::init() { WizardBase::init(); }
