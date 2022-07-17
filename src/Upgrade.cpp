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

    const float w = mDragComp->rect.h() / 2;
    const float TWO_PI = 2 * M_PI;

    float a = (mDragComp->rect.w() - w) / 2;
    float b = (mDragComp->rect.h() - w) / 2;

    const float HALF_PI = M_PI / 2;
    const int NUM_STEPS = floor(mDragComp->rect.w() / w);
    const float STEP = M_PI / NUM_STEPS;
    const float ERR = 1e-5;

    float theta = fmod(
        mScroll * TWO_PI / (mDragComp->rect.w() * 2) + 3 * HALF_PI, TWO_PI);
    float minTheta = fmod((theta - HALF_PI), STEP);
    if (minTheta <= STEP - minTheta) {
        minTheta += HALF_PI;
    } else {
        minTheta = HALF_PI - STEP + minTheta;
    }

    float cX = mDragComp->rect.halfW();
    float cY = mDragComp->rect.halfH() - w / 4;

    std::vector<float> back, front;

    for (int i = 0; i <= NUM_STEPS; i++) {
        int j = i;
        do {
            float angle = fmod(minTheta + j * STEP + TWO_PI, TWO_PI);
            if (angle < M_PI) {
                back.push_back(angle);
            } else {
                front.push_back(angle);
            }
            j *= -1;
        } while (j == -i && i != 0 && i != NUM_STEPS);
    }

    auto drawAngle = [this, HALF_PI, TWO_PI, ERR, theta, w, cX, cY, a,
                      b](float angle) {
        float angleDiff1 = fmod(angle + 3 * HALF_PI, TWO_PI);
        float angleDiff2 = fmod(5 * HALF_PI - angle, TWO_PI);
        float angleDiff = fmin(angleDiff1, angleDiff2);
        Rect rect(0, 0, w * angleDiff / M_PI, w * angleDiff / M_PI);
        if (rect.empty()) {
            return;
        }

        float x = cX + a * cos(angle);
        float y = cY - b * sin(angle);
        rect.setPos(x, y, Rect::Align::CENTER);

        angleDiff = fmod(angle - theta + TWO_PI + ERR, TWO_PI);
        uint8_t color = 255 * (1 - angleDiff / TWO_PI);
        RectData rd;
        rd.color = {color, 0, 0, 255};
        mTex.draw(rd.set(rect));
    };

    for (float angle : back) {
        drawAngle(angle);
    }

    for (float angle : front) {
        drawAngle(angle);
    }

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
