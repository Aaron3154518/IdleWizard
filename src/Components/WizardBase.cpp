#include "WizardBase.h"

// WizardBase
const Rect WizardBase::IMG_RECT(0, 0, 100, 100);
const FontData WizardBase::FONT{-1, IMG_RECT.H() / 4, "|"};
const AnimationData WizardBase::STAR_IMG{"res/wizards/star_ss.png", 6, 125};

WizardBase::WizardBase(WizardId id)
    : mId(id),
      mShowStar(false),
      mPos(std::make_shared<UIComponent>(Rect(), Elevation::WIZARDS)),
      mDrag(std::make_shared<DragComponent>(250)) {}
WizardBase::~WizardBase() {}

void WizardBase::init() {
    SDL_Point screenDim = RenderSystem::getWindowSize();
    setPos((rDist(gen) * 3 + 2) * screenDim.x / 6,
           (rDist(gen) * 3 + 2) * screenDim.y / 6);

    mStar.set(STAR_IMG);

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
    mStarTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            [this](Timer& t) { return onStarTimer(t); }, Timer(250));
    mStarAnimSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            mStar->nextFrame();
            return true;
        },
        STAR_IMG.frame_ms);
    mHideSub = WizardSystem::GetHideObservable()->subscribe(
        [this](bool hide) { onHide(hide); }, mId);
    attachSubToVisibility(mResizeSub);
    attachSubToVisibility(mRenderSub);
    attachSubToVisibility(mMouseSub);
    attachSubToVisibility(mDragSub);

    setSubscriptions();
    setUpgrades();
    setParamTriggers();

    GetWizardUpgradesObservable()->next(mId, mUpgrades);
}

void WizardBase::setSubscriptions() {}
void WizardBase::setUpgrades() {}
void WizardBase::setParamTriggers() {}

void WizardBase::onResize(ResizeData data) {
    setPos(mPos->rect.cX() * data.newW / data.oldW,
           mPos->rect.cY() * data.newH / data.oldH);
}

void WizardBase::onRender(SDL_Renderer* r) {
    TextureBuilder tex;

    if (mDrag->dragging) {
        auto rect = RectShape(GRAY).set(mPos->rect, 5);
        tex.draw(rect);
    }

    tex.draw(mImg);

    if (mShowStar) {
        float w = fmaxf(mPos->rect.w(), mPos->rect.h()) / 3;
        mStar.setDest(Rect(mPos->rect.x(), mPos->rect.y(), w, w));
        tex.draw(mStar);
    }
}

void WizardBase::onClick(Event::MouseButton b, bool clicked) {
    if (clicked) {
        mShowStar = false;
        if (!mUpgrades->isOpen()) {
            showUpgrades();
        }
    }
}

bool WizardBase::onStarTimer(Timer& t) {
    if (mShowStar) {
        mActiveUps = mUpgrades->getSnapshot();
    } else {
        auto active = mUpgrades->getSnapshot();
        for (auto pair : active) {
            auto it = mActiveUps.find(pair.first);
            if (it == mActiveUps.end() || it->second != pair.second) {
                showStar();
                break;
            }
        }
        mActiveUps = active;
    }
    return true;
}

void WizardBase::onHide(bool hide) {
    mHidden = hide;
    mPos->visible = !mHidden;
    for (auto wSub : mVisibilitySubs) {
        auto sub = wSub.lock();
        if (sub) {
            sub->setActive(!hide);
        }
    }

    if (!mHidden) {
        mActiveUps.clear();
    }
}

void WizardBase::showUpgrades() {
    ServiceSystem::Get<UpgradeService, UpgradeListObservable>()->next(
        mUpgrades, IconSystem::Get(Definitions::WIZ_IMG(mId)));
}

void WizardBase::setPos(float x, float y) {
    SDL_Point screenDim = RenderSystem::getWindowSize();
    Rect imgR = mImg.getRect();
    imgR.setPos(x, y, Rect::Align::CENTER);
    imgR.fitWithin(Rect(0, 0, screenDim.x, screenDim.y));
    mImg.setDest(imgR);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardPosObservable()->next(mId, mPos->rect);
}

void WizardBase::showStar() { mShowStar = true; }

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
