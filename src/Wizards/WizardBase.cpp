#include "WizardBase.h"

// WizardBase
const Rect WizardBase::IMG_RECT(0, 0, 100, 100);
const FontData WizardBase::FONT{-1, IMG_RECT.H() / 4, "|"};

WizardBase::WizardBase(WizardId id)
    : mId(id),
      mPos(std::make_shared<UIComponent>(Rect(), Elevation::WIZARDS)),
      mDrag(std::make_shared<DragComponent>(250)) {}
WizardBase::~WizardBase() {}

void WizardBase::init() {
    setImage(WIZ_IMGS.at(mId));
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

void WizardBase::onClick(Event::MouseButton b, bool clicked) {
    if (clicked) {
        ServiceSystem::Get<UpgradeService, UpgradeListObservable>()->next(
            mUpgrades);
    }
}

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
