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
        scroll(-mScrollV * dt.s());

        mScrollV /= pow(100, dt.s());
        if (abs(mScrollV) <= 1) {
            mScrollV = 0;
        }
    } else {
        mScrollV /= dt.s();
    }
}
void UpgradeScroller::onRender(SDL_Renderer* r) {
    mTexData.dest = mDragComp->rect;
    TextureBuilder().draw(mTexData);
}
void UpgradeScroller::onClick(Event::MouseButton b, bool clicked) {}
void UpgradeScroller::onDrag(int mouseX, int mouseY, float mouseDx,
                             float mouseDy) {
    scroll(-mouseDx);
    mScrollV = mouseDx;
}
void UpgradeScroller::onDragStart() { mScrollV = 0; }

void UpgradeScroller::scroll(float dScroll) {
    mScroll = fmax(fmin(mScroll + dScroll, maxScroll()), 0);
    draw();
}
float UpgradeScroller::maxScroll() const {
    return mDragComp->rect.w() * (mUpgrades.size() - 1) /
           floor(mDragComp->rect.w() * 2 / mDragComp->rect.h());
}

void UpgradeScroller::draw() {
    mTex.draw(mBkgrnd);

    const float w = mDragComp->rect.h() / 2;
    float a = (mDragComp->rect.w() - w) / 2;
    float b = (mDragComp->rect.h() - w) / 2;

    const float TWO_PI = 2 * M_PI;
    const float HALF_PI = M_PI / 2;
    const int NUM_STEPS = floor(mDragComp->rect.w() / w);
    const float STEP = M_PI / NUM_STEPS;
    const float ERR = 1e-5;

    // Compute total scroll angle
    float scrollAngle = mScroll * TWO_PI / (mDragComp->rect.w() * 2);
    // Check number of steps passed, Use .5 - ERR so we don't round up at .5
    int baseIdx = floor(scrollAngle / STEP + .5 - ERR);
    // Constrain scroll angle to [0, 2PI)
    scrollAngle = fmod(scrollAngle, TWO_PI);
    // Transform to CCW with 0 at 3PI/2
    float theta = fmod(5 * HALF_PI - scrollAngle, TWO_PI);
    // Find the step closest to PI/2
    float minTheta = fmod((theta + 3 * HALF_PI), STEP);
    if (minTheta + ERR < STEP - minTheta) {
        minTheta += HALF_PI;
    } else {
        minTheta = HALF_PI - STEP + minTheta;
    }

    float cX = mDragComp->rect.halfW();
    float cY = mDragComp->rect.halfH() - w / 4;

    // Pair <angle, index>
    std::forward_list<std::pair<float, int>> back, front;

    for (int i = NUM_STEPS; i >= 0; i--) {
        int sign = 1;
        do {
            float angle = fmod(minTheta + i * sign * STEP + TWO_PI, TWO_PI);
            int idx = baseIdx - (NUM_STEPS - i) * sign;
            if (idx >= 0 && idx < mUpgrades.size()) {
                std::pair<float, int> pair = std::make_pair(angle, idx);
                if (angle < M_PI) {
                    back.push_front(pair);
                } else {
                    front.push_front(pair);
                }
            }
            sign *= -1;
        } while (sign == -1 && i != 0 && i != NUM_STEPS);
    }

    TextRenderData tData;
    tData.tData.font = AssetManager::getFont(WizardBase::FONT);
    auto drawAngle = [this, HALF_PI, TWO_PI, ERR, w, cX, cY, a, b, &tData](
                         float angle, int val) {
        float angleDiff1 = fmod(angle + 3 * HALF_PI, TWO_PI);
        float angleDiff2 = fmod(5 * HALF_PI - angle, TWO_PI);
        float angleDiff = fmin(angleDiff1, angleDiff2);
        Rect rect(0, 0, w * angleDiff / M_PI, w * angleDiff / M_PI);
        if (rect.Empty()) {
            return;
        }

        float x = cX + a * cos(angle);
        float y = cY - b * sin(angle);
        rect.setPos(x, y, Rect::Align::CENTER);

        tData.tData.text = std::to_string(val);
        tData.dest = rect;
        tData.renderText();
        tData.fitToTexture();
        mTex.draw(tData);
    };

    for (auto pair : back) {
        drawAngle(pair.first, mUpgrades.at(pair.second));
    }

    for (auto pair : front) {
        drawAngle(pair.first, mUpgrades.at(pair.second));
    }
}
