#include "Upgrade.h"

#include "Wizard.h"

// UpgradeScroller
UpgradeScroller::UpgradeScroller()
    : mDragComp(
          std::make_shared<DragComponent>(Rect(), Elevation::UPGRADES, -1)) {
    mDragComp->rect = Rect(0, 0, WizardBase::BORDER_RECT.w(),
                           WizardBase::BORDER_RECT.h() / 5);
    mDragComp->rect.setPosX(WizardBase::BORDER_RECT.halfW(),
                            Rect::Align::CENTER);

    mTex = TextureBuilder(mDragComp->rect.W(), mDragComp->rect.H());
    mTexData.texture = mTex.getTexture();

    mBkgrnd.color = GRAY;
}

void UpgradeScroller::init() {
    mDragComp->onDrag = std::bind(&UpgradeScroller::onDrag, this,
                                  std::placeholders::_1, std::placeholders::_2,
                                  std::placeholders::_3, std::placeholders::_4);
    mDragComp->onDragStart = std::bind(&UpgradeScroller::onDragStart, this);
    mDragComp->onDragEnd = []() {};

    mUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            std::bind(&UpgradeScroller::onUpdate, this, std::placeholders::_1));
    mUpdateSub->setUnsubscriber(unsub);
    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&UpgradeScroller::onRender, this, std::placeholders::_1),
            mDragComp);
    mRenderSub->setUnsubscriber(unsub);
    mMouseSub = ServiceSystem::Get<MouseService, MouseObservable>()->subscribe(
        std::bind(&UpgradeScroller::onClick, this, std::placeholders::_1,
                  std::placeholders::_2),
        mDragComp);
    mMouseSub->setUnsubscriber(unsub);
    mDragSub =
        ServiceSystem::Get<DragService, DragObservable>()->subscribe(mDragComp);
    mDragSub->setUnsubscriber(unsub);
}

void UpgradeScroller::onUpdate(Time dt) {
    if (!mDragComp->dragging) {
        mScroll += mScrollV * dt.s();
        mScrollV /= pow(100, dt.s());
        if (abs(mScrollV) <= 1) {
            mScrollV = 0;
        }
    } else {
        mScrollV /= dt.s();
    }
}
void UpgradeScroller::onRender(SDL_Renderer* r) {
    mTex.draw(mBkgrnd);
    RectData rd;
    mTex.draw(rd.set(Rect(mScroll, 0, 50, 50)));

    mTexData.dest = mDragComp->rect;
    TextureBuilder().draw(mTexData);
}
void UpgradeScroller::onClick(Event::MouseButton b, bool clicked) {}
void UpgradeScroller::onDrag(int mouseX, int mouseY, float mouseDx,
                             float mouseDy) {
    mScroll += mouseDx;
    mScrollV = mouseDx;
}
void UpgradeScroller::onDragStart() { mScrollV = 0; }
