#include "Upgrade.h"

#include "Wizard.h"

// Upgrade
void Upgrade::setImg(std::string img) { setImg(AssetManager::getTexture(img)); }
void Upgrade::setImg(SharedTexture img) {
    mImg.texture = img;
    setImgHandler([this]() { return mImg; });
}

void Upgrade::setImgHandler(std::function<RenderData()> func) {
    mImgHandler = mImgReply.subscribeToRequest(func);
}

void Upgrade::requestImg(std::function<void(RenderData)> func) {
    RenderReply::ResponseObservable::SubscriptionPtr tmpSub =
        mImgReply.subscribeToResponse(func);
    mImgReply.next();
}

void Upgrade::setDescriptionHandler(std::function<RenderData()> func) {
    mDescHandler = mDescReply.subscribeToRequest(func);
}

void Upgrade::requestDescription(std::function<void(RenderData)> func) {
    RenderReply::ResponseObservable::SubscriptionPtr tmpSub =
        mDescReply.subscribeToResponse(func);
    mDescReply.next();
}

// UpgradeScroller
UpgradeScroller::UpgradeScroller()
    : mPos(std::make_shared<UIComponent>(Rect(), Elevation::UPGRADES)),
      mDrag(std::make_shared<DragComponent>(-1)) {
    SDL_Point screenDim = RenderSystem::getWindowSize();
    mPos->rect = Rect(0, 0, screenDim.x,
                      fmin(screenDim.y / 5, WizardBase::IMG_RECT.h()));
    mPos->rect.setPosX(screenDim.x / 2, Rect::Align::CENTER);

    mTex = TextureBuilder(mPos->rect.W(), mPos->rect.H());
    mTexData.texture = mTex.getTexture();

    mBkgrnd.color = GRAY;
}

void UpgradeScroller::init() {
    mResizeSub =
        ServiceSystem::Get<ResizeService, ResizeObservable>()->subscribe(
            std::bind(&UpgradeScroller::onResize, this, std::placeholders::_1));
    mUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            std::bind(&UpgradeScroller::onUpdate, this, std::placeholders::_1));
    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&UpgradeScroller::onRender, this, std::placeholders::_1),
            mPos);
    mMouseSub = ServiceSystem::Get<MouseService, MouseObservable>()->subscribe(
        std::bind(&UpgradeScroller::onClick, this, std::placeholders::_1,
                  std::placeholders::_2),
        mPos);
    mDragSub = ServiceSystem::Get<DragService, DragObservable>()->subscribe(
        std::bind(&UpgradeScroller::onDragStart, this),
        std::bind(&UpgradeScroller::onDrag, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3,
                  std::placeholders::_4),
        []() {}, mPos, mDrag);
    mHoverSub = ServiceSystem::Get<HoverService, HoverObservable>()->subscribe(
        []() {},
        std::bind(&UpgradeScroller::onHover, this, std::placeholders::_1),
        std::bind(&UpgradeScroller::onMouseLeave, this), mPos);
    mUpgradeSub =
        ServiceSystem::Get<UpgradeService, UpgradeObservable>()->subscribe(
            std::bind(&UpgradeScroller::onSetUpgrades, this,
                      std::placeholders::_1));
}

void UpgradeScroller::onResize(ResizeData data) {
    mPos->rect =
        Rect(0, 0, data.newW, fmin(data.newH / 5, WizardBase::IMG_RECT.h()));
    mPos->rect.setPosX(data.newW / 2, Rect::Align::CENTER);

    mTex = TextureBuilder(mPos->rect.W(), mPos->rect.H());
    mTexData.texture = mTex.getTexture();

    computeRects();
}
void UpgradeScroller::onUpdate(Time dt) {
    if (!mDrag->dragging) {
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
    draw();

    // Draw texture
    mTexData.dest = mPos->rect;
    TextureBuilder().draw(mTexData);

    // Draw uprade description
}
void UpgradeScroller::onClick(Event::MouseButton b, bool clicked) {
    if (clicked) {
        for (auto pair : mFrontRects) {
            if (SDL_PointInRect(&b.clickPos, pair.first)) {
                auto up = mUpgrades.at(pair.second).lock();
                if (!up) {
                    draw();
                } else if (up->status() == Upgrade::Status::CAN_BUY) {
                    up->onClick();
                }
                return;
            }
        }
        for (auto pair : mBackRects) {
            if (SDL_PointInRect(&b.clickPos, pair.first)) {
                auto up = mUpgrades.at(pair.second).lock();
                if (!up) {
                    draw();
                } else if (up->status() == Upgrade::Status::CAN_BUY) {
                    up->onClick();
                }
                return;
            }
        }
    }
}
void UpgradeScroller::onDrag(int mouseX, int mouseY, float mouseDx,
                             float mouseDy) {
    scroll(-mouseDx);
    mScrollV = mouseDx;
}
void UpgradeScroller::onDragStart() { mScrollV = 0; }
void UpgradeScroller::onHover(SDL_Point mouse) {
    mUpDescRenderSub.reset();
    auto onDescription = [this, &mouse](RenderData rData) {
        rData.fitToTexture();
        rData.dest.setPos(mouse.x, mouse.y);
        mUpDescRenderSub =
            ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
                [rData](SDL_Renderer* r) { TextureBuilder().draw(rData); },
                std::make_shared<UIComponent>(rData.dest, Elevation::OVERLAYS));
        mUpDescRenderSub->get<RenderObservable::DATA>()->mouse = false;
    };

    SDL_Point relMouse{mouse.x - mPos->rect.X(), mouse.y - mPos->rect.Y()};
    for (auto pair : mFrontRects) {
        if (SDL_PointInRect(&relMouse, pair.first)) {
            auto upgrade = mUpgrades[pair.second].lock();
            if (upgrade) {
                upgrade->requestDescription(onDescription);
            }
            return;
        }
    }
    for (auto pair : mBackRects) {
        if (SDL_PointInRect(&relMouse, pair.first)) {
            auto upgrade = mUpgrades[pair.second].lock();
            if (upgrade) {
                upgrade->requestDescription(onDescription);
            }
            return;
        }
    }
}
void UpgradeScroller::onMouseLeave() { mUpDescRenderSub.reset(); }
void UpgradeScroller::onSetUpgrades(const UpgradeList& list) {
    mUpgrades.resize(list.size());
    for (int i = 0; i < list.size(); i++) {
        mUpgrades.at(i) = list.at(i);
    }
    scroll(0);
}

void UpgradeScroller::scroll(float dScroll) {
    mScroll = fmax(fmin(mScroll + dScroll, maxScroll()), 0);
    computeRects();
}
float UpgradeScroller::maxScroll() const {
    return mPos->rect.w() * (mUpgrades.size() - 1) /
           floor(mPos->rect.w() * 2 / mPos->rect.h());
}

void UpgradeScroller::computeRects() {
    for (auto it = mUpgrades.begin(); it != mUpgrades.end(); ++it) {
        if (!it->lock()) {
            it = mUpgrades.erase(it);
            if (it == mUpgrades.end()) {
                break;
            }
        }
    }

    const float w = mPos->rect.h() / 2;
    float a = (mPos->rect.w() - w) / 2;
    float b = (mPos->rect.h() - w) / 2;

    const float TWO_PI = 2 * M_PI;
    const float HALF_PI = M_PI / 2;
    const int NUM_STEPS = floor(mPos->rect.w() / w);
    const float STEP = M_PI / NUM_STEPS;
    const float ERR = 1e-5;

    // Compute total scroll angle
    float scrollAngle = mScroll * TWO_PI / (mPos->rect.w() * 2);
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

    float cX = mPos->rect.halfW();
    float cY = mPos->rect.halfH() - w / 4;

    // Pair <angle, index>
    std::forward_list<std::pair<float, int>> back, front;
    int backLen = 0, frontLen = 0;

    for (int i = NUM_STEPS; i >= 0; i--) {
        int sign = 1;
        do {
            float angle = fmod(minTheta + i * sign * STEP + TWO_PI, TWO_PI);
            int idx = baseIdx - (NUM_STEPS - i) * sign;
            if (idx >= 0 && idx < mUpgrades.size()) {
                std::pair<float, int> pair = std::make_pair(angle, idx);
                if (angle < M_PI) {
                    back.push_front(pair);
                    backLen++;
                } else {
                    front.push_front(pair);
                    frontLen++;
                }
            }
            sign *= -1;
        } while (sign == -1 && i != 0 && i != NUM_STEPS);
    }

    auto getRect = [HALF_PI, TWO_PI, ERR, w, cX, cY, a,
                    b](float angle) -> Rect {
        float angleDiff1 = fmod(angle + 3 * HALF_PI, TWO_PI);
        float angleDiff2 = fmod(5 * HALF_PI - angle, TWO_PI);
        float angleDiff = fmin(angleDiff1, angleDiff2);
        Rect rect(0, 0, w * angleDiff / M_PI, w * angleDiff / M_PI);

        float x = cX + a * cos(angle);
        float y = cY - b * sin(angle);
        rect.setPos(x, y, Rect::Align::CENTER);

        return rect;
    };

    mBackRects = std::vector<std::pair<Rect, int>>(backLen);
    mFrontRects = std::vector<std::pair<Rect, int>>(frontLen);
    int i = 0;
    for (auto pair : back) {
        mBackRects.at(i++) = std::make_pair(getRect(pair.first), pair.second);
    }

    i = 0;
    for (auto pair : front) {
        mFrontRects.at(i++) = std::make_pair(getRect(pair.first), pair.second);
    }
}
void UpgradeScroller::draw() {
    mTex.draw(mBkgrnd);

    auto drawUpgrade = [this](Rect r, std::shared_ptr<Upgrade> up) {
        RectData rd;
        if (!up) {
            rd.color = WHITE;
        } else {
            switch (up->status()) {
                case Upgrade::Status::BOUGHT:
                    rd.color = BLUE;
                    break;
                case Upgrade::Status::CAN_BUY:
                    rd.color = GREEN;
                    break;
                case Upgrade::Status::CANT_BUY:
                    rd.color = RED;
                    break;
            }
        }
        mTex.draw(rd.set(r, 3));

        if (up) {
            up->requestImg([this, &r](RenderData rData) {
                rData.dest = r;
                rData.fitToTexture();
                mTex.draw(rData);
            });
        }
    };

    for (auto pair : mBackRects) {
        drawUpgrade(pair.first, mUpgrades.at(pair.second).lock());
    }

    for (auto pair : mFrontRects) {
        drawUpgrade(pair.first, mUpgrades.at(pair.second).lock());
    }
}
