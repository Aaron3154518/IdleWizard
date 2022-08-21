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

    setDefaultValues();
    setSubscriptions();
    setUpgrades();
    setParamTriggers();
    setEventTriggers();
}

void WizardBase::setDefaultValues() {}
void WizardBase::setSubscriptions() {
    mResizeSub =
        ServiceSystem::Get<ResizeService, ResizeObservable>()->subscribe(
            [this](ResizeData data) { onResize(data); });
    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [this](SDL_Renderer* r) { onRender(r); }, mPos);
    mMouseSub = ServiceSystem::Get<MouseService, MouseObservable>()->subscribe(
        [this](Event::MouseButton b, bool clicked) { onClick(b, clicked); },
        mPos);
    mDragSub = ServiceSystem::Get<DragService, DragObservable>()->subscribe(
        []() {}, [this](int x, int y, float dx, float dy) { setPos(x, y); },
        []() {}, mPos, mDrag);
    mHideSub = WizardSystem::GetHideObservable()->subscribe(
        [this](WizardId id, bool hide) { onHide(id, hide); });
    attachSubToVisibility(mResizeSub);
    attachSubToVisibility(mRenderSub);
    attachSubToVisibility(mMouseSub);
    attachSubToVisibility(mDragSub);
}
void WizardBase::setUpgrades() {}
void WizardBase::setParamTriggers() {}
void WizardBase::setEventTriggers() {}

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

void WizardBase::onHide(WizardId id, bool hide) {
    if (id == mId) {
        mHidden = hide;
        mPos->visible = !mHidden;
        for (auto wSub : mVisibilitySubs) {
            auto sub = wSub.lock();
            if (sub) {
                sub->setActive(!hide);
            }
        }
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
    mImg.shrinkToTexture();
    mPos->rect = mImg.dest;
}

void WizardBase::attachSubToVisibility(SubscriptionBaseWPtr wSub) {
    mVisibilitySubs.push_back(wSub);
    auto sub = wSub.lock();
    if (sub) {
        sub->setActive(!mHidden);
    }
}
void WizardBase::detachSubFromVisibility(SubscriptionBasePtr sub) {
    auto it = std::find_if(mVisibilitySubs.begin(), mVisibilitySubs.end(),
                           [&sub](const SubscriptionBaseWPtr wSub) {
                               return wSub.lock().get() == sub.get();
                           });
    if (it != mVisibilitySubs.end()) {
        mVisibilitySubs.erase(it);
    }
}
