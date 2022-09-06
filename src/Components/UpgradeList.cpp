#include "UpgradeList.h"

// UpgradeList
int UpgradeList::size() const { return getNumActive(); }

void UpgradeList::onClick(SDL_Point mouse) {
    for (auto pair : mFrontRects) {
        if (SDL_PointInRect(&mouse, pair.first)) {
            auto sub = pair.second.lock();
            if (sub) {
                sub->get<DATA>()->buy();
            }
            return;
        }
    }
    for (auto pair : mBackRects) {
        if (SDL_PointInRect(&mouse, pair.first)) {
            auto sub = pair.second.lock();
            if (sub) {
                sub->get<DATA>()->buy();
            }
            return;
        }
    }
}

RenderObservable::SubscriptionPtr UpgradeList::onHover(SDL_Point mouse,
                                                       SDL_Point relMouse) {
    for (auto pair : mFrontRects) {
        if (SDL_PointInRect(&relMouse, pair.first)) {
            auto weakSub = pair.second;
            return ServiceSystem::Get<RenderService, RenderObservable>()
                ->subscribe(
                    [mouse, pair](SDL_Renderer* r) {
                        auto sub = pair.second.lock();
                        if (sub) {
                            sub->get<UpgradeList::DATA>()->drawDescription(
                                TextureBuilder(),
                                pair.first.getPos(Rect::CENTER,
                                                  Rect::BOT_RIGHT));
                        }
                    },
                    std::make_shared<UIComponent>(Rect(), Elevation::OVERLAYS));
        }
    }
    for (auto pair : mBackRects) {
        if (SDL_PointInRect(&relMouse, pair.first)) {
            auto weakSub = pair.second;
            return ServiceSystem::Get<RenderService, RenderObservable>()
                ->subscribe(
                    [mouse, pair](SDL_Renderer* r) {
                        auto sub = pair.second.lock();
                        if (sub) {
                            sub->get<UpgradeList::DATA>()->drawDescription(
                                TextureBuilder(),
                                pair.first.getPos(Rect::CENTER,
                                                  Rect::BOT_RIGHT));
                        }
                    },
                    std::make_shared<UIComponent>(Rect(), Elevation::OVERLAYS));
        }
    }
    return nullptr;
}

void UpgradeList::draw(TextureBuilder tex, float scroll, SDL_Point offset) {
    bool recompute = false;
    if (getNumActive() != mCount) {
        mCount = getNumActive();
        recompute = true;
    }
    if (mScroll != scroll) {
        mScroll = scroll;
        recompute = true;
    }
    SDL_Point dim = tex.getTextureSize();
    Rect r(0, 0, dim.x, dim.y);
    if (mRect != r) {
        mRect = r;
        recompute = true;
    }
    if (recompute) {
        computeRects();
    } else {
        // Upgrade order may have changed so reassign to rects
        int i = 0;
        for (auto sub : *this) {
            auto map = mIdxMap.at(i++);
            (map.first ? mFrontRects : mBackRects).at(map.second).second = sub;
        }
    }

    auto drawUpgrade = [this, &tex, offset](Rect r, SubscriptionPtr sub) {
        r.move(offset.x, offset.y);
        RectShape rd;
        if (!sub) {
            rd.mColor = WHITE;
            tex.draw(rd.set(r, 3));
        } else {
            auto up = sub->get<DATA>();
            switch (up->getStatus()) {
                case Upgrade::Status::BOUGHT:
                    rd.mColor = BLUE;
                    break;
                case Upgrade::Status::CAN_BUY:
                    rd.mColor = GREEN;
                    break;
                case Upgrade::Status::CANT_BUY:
                    rd.mColor = RED;
                    break;
                case Upgrade::Status::NOT_BUYABLE:
                    rd.mColor = BLACK;
                    break;
            }
            tex.draw(rd.set(r, 3));
            up->drawIcon(tex, r);
        }
    };

    for (auto pair : mBackRects) {
        drawUpgrade(pair.first, pair.second.lock());
    }

    for (auto pair : mFrontRects) {
        drawUpgrade(pair.first, pair.second.lock());
    }
}

void UpgradeList::computeRects() {
    prune();

    const float w = mRect.h() / 2;
    float a = (mRect.w() - w) / 2;
    float b = (mRect.h() - w) / 2;

    const float TWO_PI = 2 * M_PI;
    const float HALF_PI = M_PI / 2;
    const int NUM_STEPS = floor(mRect.w() / w);
    const float STEP = M_PI / NUM_STEPS;
    const float ERR = 1e-5;

    // Compute total scroll angle
    float scrollAngle = mScroll * TWO_PI / (mRect.w() * 2);
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

    float cX = mRect.halfW();
    float cY = mRect.halfH() - w / 4;

    int num = getNumActive();
    std::vector<float> rectAngles(num);
    int backLen = 0, frontLen = 0;

    for (int i = NUM_STEPS; i >= 0; i--) {
        int sign = 1;
        do {
            float angle = fmod(minTheta + i * sign * STEP + TWO_PI, TWO_PI);
            int idx = baseIdx - (NUM_STEPS - i) * sign;
            if (idx >= 0 && idx < getNumActive()) {
                rectAngles[idx] = angle;
                if (angle < M_PI) {
                    backLen++;
                } else {
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

    mBackRects.resize(backLen);
    mFrontRects.resize(frontLen);
    mIdxMap.resize(backLen + frontLen);
    int i = 0, bIdx = 0, fIdx = 0;
    for (auto sub : *this) {
        float angle = rectAngles[i];
        bool back = angle < M_PI;
        mIdxMap.at(i) = std::make_pair(!back, back ? bIdx : fIdx);
        (back ? mBackRects.at(bIdx++) : mFrontRects.at(fIdx++)) =
            std::make_pair(getRect(angle), sub);
        i++;
    }
}

std::unordered_set<void*> UpgradeList::getActive() const {
    std::unordered_set<void*> set;
    for (auto sub : *this) {
        set.insert(sub.get());
    }
    return set;
}

void UpgradeList::buyAll(ParameterSystem::BaseValue money, Number max) {
    bool noMax = max < 0;

    for (auto sub : *this) {
        auto& up = sub->get<DATA>();
        const auto& upCost = up->getCost();
        if (upCost && upCost->getMoneyParam() == money) {
            while (up->getStatus() == Upgrade::CAN_BUY &&
                   (noMax || upCost->getCost() <= max)) {
                if (!noMax) {
                    max -= upCost->getCost();
                }
                up->buy();
            }
        }
    }
}

// WizardUpgradesObservable
const UpgradeListPtr& GetWizardUpgrades(WizardId id) {
    return GetWizardUpgradesObservable()->get(id);
}

std::shared_ptr<WizardUpgradesObservable> GetWizardUpgradesObservable() {
    return ServiceSystem::Get<UpgradeService, WizardUpgradesObservable>();
}

// UpgradeScroller
const SDL_Color UpgradeScroller::BKGRND = GRAY;
const Rect UpgradeScroller::RECT(0, 0, 100, 100);

UpgradeScroller::UpgradeScroller()
    : mPos(std::make_shared<UIComponent>(Rect(), Elevation::UPGRADES)),
      mDrag(std::make_shared<DragComponent>(-1)) {
    SDL_Point screenDim = RenderSystem::getWindowSize();
    mPos->rect = Rect(0, 0, screenDim.x, fmin(screenDim.y / 5, RECT.h()));
    mPos->rect.setPosX(screenDim.x / 2, Rect::Align::CENTER);

    mTex = TextureBuilder(mPos->rect.W(), mPos->rect.H());
    mTexData.set(mTex.getTexture());
}

void UpgradeScroller::init() {
    mResizeSub =
        ServiceSystem::Get<ResizeService, ResizeObservable>()->subscribe(
            [this](ResizeData d) { onResize(d); });
    mUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            [this](Time dt) { onUpdate(dt); });
    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [this](SDL_Renderer* r) { onRender(r); }, mPos);
    mMouseSub = ServiceSystem::Get<MouseService, MouseObservable>()->subscribe(
        [this](Event::MouseButton b, bool c) { onClick(b, c); }, mPos);
    mDragSub = ServiceSystem::Get<DragService, DragObservable>()->subscribe(
        [this]() { onDragStart(); },
        [this](float x, float y, float dx, float dy) { onDrag(x, y, dx, dy); },
        []() {}, mPos, mDrag);
    mHoverSub = ServiceSystem::Get<HoverService, HoverObservable>()->subscribe(
        []() {}, [this](SDL_Point m) { onHover(m); },
        [this]() { onMouseLeave(); }, mPos);
    mUpgradeSub =
        ServiceSystem::Get<UpgradeService, UpgradeListObservable>()->subscribe(
            [this](UpgradeListPtr us) { onSetUpgrades(us); });
}

void UpgradeScroller::onResize(ResizeData data) {
    mPos->rect = Rect(0, 0, data.newW, fmin(data.newH / 5, RECT.h()));
    mPos->rect.setPosX(data.newW / 2, Rect::Align::CENTER);

    mTex = TextureBuilder(mPos->rect.W(), mPos->rect.H());
    mTexData.set(mTex.getTexture());
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
    RectShape rect(BKGRND);
    mTex.draw(rect);

    if (mUpgrades) {
        mUpgrades->draw(mTex, mScroll);
    }

    // Draw texture to screen
    mTexData.setDest(mPos->rect);
    TextureBuilder().draw(mTexData);
}
void UpgradeScroller::onClick(Event::MouseButton b, bool clicked) {
    if (clicked && mUpgrades) {
        mUpgrades->onClick(b.clickPos);
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

    if (mUpgrades) {
        mUpDescRenderSub = mUpgrades->onHover(
            mouse, {mouse.x - mPos->rect.X(), mouse.y - mPos->rect.Y()});
        if (mUpDescRenderSub) {
            mUpDescRenderSub->get<RenderObservable::DATA>()->mouse = false;
        }
    }
}
void UpgradeScroller::onMouseLeave() { mUpDescRenderSub.reset(); }
void UpgradeScroller::onSetUpgrades(UpgradeListPtr upgrades) {
    mUpgrades = upgrades;
    scroll(0);
}

void UpgradeScroller::scroll(float dScroll) {
    mScroll = fmax(fmin(mScroll + dScroll, maxScroll()), 0);
}
float UpgradeScroller::maxScroll() const {
    return mPos->rect.w() * (mUpgrades ? mUpgrades->size() - 1 : 0) /
           floor(mPos->rect.w() * 2 / mPos->rect.h());
}
