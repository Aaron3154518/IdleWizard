#include "UpgradeList.h"

// UpgradeList
int UpgradeList::size() const { return getNumActive(); }

UpgradeActiveList UpgradeList::getSnapshot() const {
    UpgradeActiveList list;
    for (auto sub : *this) {
        list[sub.get()] = {sub->get<DATA>()->getSnapshot()};
    }
    return list;
}

bool UpgradeList::canBuyOne(ParameterSystem::BaseValue money, Number max) {
    bool noMax = max < 0;

    for (auto sub : *this) {
        auto& up = sub->get<DATA>();
        const auto& upCost = up->getCost();
        if (upCost && upCost->getMoneyParam() == money) {
            if (up->status(true) != Upgrade::Status::BOUGHT &&
                (noMax || upCost->getCost() <= max)) {
                return true;
            }
        }
    }

    return false;
}
Number UpgradeList::upgradeAll(ParameterSystem::BaseValue money, Number max) {
    bool noMax = max < 0;
    Number cost = 0;

    for (auto sub : *this) {
        auto& up = sub->get<DATA>();
        const auto& upCost = up->getCost();
        if (upCost && upCost->getMoneyParam() == money) {
            while (up->status(true) != Upgrade::Status::BOUGHT &&
                   (noMax || cost + upCost->getCost() <= max)) {
                cost += upCost->getCost();
                up->buy(true);
            }
        }
    }

    return cost;
}
void UpgradeList::maxAll(ParameterSystem::BaseValue money) {
    for (auto sub : *this) {
        auto& up = sub->get<DATA>();
        const auto& upCost = up->getCost();
        if (upCost && upCost->getMoneyParam() == money) {
            up->max(true);
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

// UpgradeRenderer
void UpgradeRenderer::onClick(SDL_Point mouse) {}
RenderObservable::SubscriptionPtr UpgradeRenderer::onHover(SDL_Point mouse,
                                                           SDL_Point relMouse) {
    return nullptr;
}

void UpgradeRenderer::draw(TextureBuilder tex, float scroll, SDL_Point offset) {
}

float UpgradeRenderer::minScroll() const { return 0; }
float UpgradeRenderer::maxScroll() const { return 0; }

// UpgradeScroller
UpgradeScroller::UpgradeScroller(UpgradeListPtr upgrades)
    : mUpgrades(upgrades) {}

void UpgradeScroller::onClick(SDL_Point mouse) {
    for (auto& vec : {mFrontRects, mBackRects}) {
        for (auto pair : vec) {
            if (SDL_PointInRect(&mouse, pair.first)) {
                auto sub = pair.second.lock();
                if (sub) {
                    sub->get<UpgradeList::DATA>()->buy();
                }
                return;
            }
        }
    }
}

RenderObservable::SubscriptionPtr UpgradeScroller::onHover(SDL_Point mouse,
                                                           SDL_Point relMouse) {
    for (auto& vec : {mFrontRects, mBackRects}) {
        for (auto pair : vec) {
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
                        std::make_shared<UIComponent>(Rect(),
                                                      Elevation::OVERLAYS));
            }
        }
    }
    return nullptr;
}

void UpgradeScroller::draw(TextureBuilder tex, float scroll, SDL_Point offset) {
    bool recompute = false;
    if (mUpgrades->getNumActive() != mCount) {
        mCount = mUpgrades->getNumActive();
        recompute = true;
    }
    if (mScroll != scroll) {
        mScroll = scroll;
        recompute = true;
    }
    SDL_Point dim = tex.getTextureSize();
    if (mDim.x != dim.x || mDim.y != dim.y) {
        mDim = dim;
        recompute = true;
    }
    if (recompute) {
        computeRects();
    } else {
        // Upgrade order may have changed so reassign to rects
        int i = 0;
        for (auto sub : *mUpgrades) {
            auto map = mIdxMap.at(i++);
            (map.first ? mFrontRects : mBackRects).at(map.second).second = sub;
        }
    }

    auto drawUpgrade = [this, &tex, offset](Rect r,
                                            UpgradeList::SubscriptionPtr sub) {
        r.move(offset.x, offset.y);
        RectShape rd;
        if (!sub) {
            rd.mColor = WHITE;
            tex.draw(rd.set(r, 3));
        } else {
            auto up = sub->get<UpgradeList::DATA>();
            switch (up->status()) {
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

float UpgradeScroller::maxScroll() const {
    if (mDim.x * mDim.y == 0) {
        return 0;
    }
    return mDim.x * (mUpgrades ? mUpgrades->size() - 1 : 0) /
           floor(mDim.x * 2 / mDim.y);
}

void UpgradeScroller::computeRects() {
    mUpgrades->prune();

    const float w = mDim.y / 2;
    float a = (mDim.x - w) / 2;
    float b = (mDim.y - w) / 2;

    const float TWO_PI = 2 * M_PI;
    const float HALF_PI = M_PI / 2;
    const int NUM_STEPS = floor(mDim.x / w);
    const float STEP = M_PI / NUM_STEPS;
    const float ERR = 1e-5;

    // Compute total scroll angle
    float scrollAngle = mScroll * TWO_PI / (mDim.x * 2);
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

    float cX = mDim.x / 2;
    float cY = mDim.y / 2 - w / 4;

    int num = mUpgrades->getNumActive();
    std::vector<float> rectAngles(num);
    int backLen = 0, frontLen = 0;

    for (int i = NUM_STEPS; i >= 0; i--) {
        int sign = 1;
        do {
            float angle = fmod(minTheta + i * sign * STEP + TWO_PI, TWO_PI);
            int idx = baseIdx - (NUM_STEPS - i) * sign;
            if (idx >= 0 && idx < num) {
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
    for (auto sub : *mUpgrades) {
        float angle = rectAngles[i];
        bool back = angle < M_PI;
        mIdxMap.at(i) = std::make_pair(!back, back ? bIdx : fIdx);
        (back ? mBackRects.at(bIdx++) : mFrontRects.at(fIdx++)) =
            std::make_pair(getRect(angle), sub);
        i++;
    }
}

// UpgradeProgressBar
UpgradeProgressBar::UpgradeProgressBar(UpgradeListPtr upgrades,
                                       ParameterSystem::ValueParam val)
    : mUpgrades(upgrades), mValParam(val) {}

RenderObservable::SubscriptionPtr UpgradeProgressBar::onHover(
    SDL_Point mouse, SDL_Point relMouse) {
    return nullptr;
}

constexpr float VAL = 100;

void UpgradeProgressBar::draw(TextureBuilder tex, float scroll,
                              SDL_Point offset) {
    // Get upgrades with log costs
    mCostVals.clear();
    for (auto sub : *mUpgrades) {
        auto up = sub->get<UpgradeList::DATA>();
        mCostVals.push_back(
            std::make_pair(toValue(up->getCost()->getCost()), sub));
    }

    // Sort by increasing cost
    std::stable_sort(mCostVals.begin(), mCostVals.end(),
                     [](const UpgradeCost& lhs, const UpgradeCost& rhs) {
                         return lhs.first < rhs.first;
                     });

    SDL_Point dim = tex.getTextureSize();
    int marginX = dim.x / 20;
    int marginY = dim.y / 10;
    mBounds = Rect(marginX, marginY, dim.x - marginX * 2, dim.y - marginY * 2);
    float pbH = mBounds.h() / 5;
    float upW = (mBounds.h() - pbH) / 2;
    mScrollMargin = upW / 2;

    scroll -= mScrollMargin;
    float start = scroll / VAL;
    float val_start = fmaxf(start, 0);
    float end = start + mBounds.w() / VAL;
    float val_end = fminf(end, mCostVals.back().first);
    float amnt = toValue(mValParam.get());

    ProgressBar pb;
    pb.set(Rect(mBounds.x() + VAL * (val_start - start), mBounds.y() + upW,
                VAL * (val_end - val_start), pbH))
        .set(WHITE, BLACK)
        .set(amnt - val_start, val_end - val_start);
    tex.draw(pb);

    RectShape rs;
    rs.set(mBounds, 2);
    tex.draw(rs);

    for (int i = 0; i < mCostVals.size(); i++) {
        auto& pair = mCostVals.at(i);
        if (fabsf(pair.first - start) < mScrollMargin ||
            fabsf(pair.first - end) < mScrollMargin) {
            auto upPtr = pair.second.lock();
            if (upPtr) {
                Rect r(0, 0, upW, upW);
                r.setPos(mBounds.x() + VAL * (pair.first - start),
                         i % 2 == 0 ? mBounds.y() : mBounds.y2() - r.h(),
                         Rect::Align::CENTER, Rect::Align::TOP_LEFT);
                auto up = upPtr->get<UpgradeList::DATA>();
                up->drawIcon(tex, r);
            }
        }
    }
}

float UpgradeProgressBar::maxScroll() const {
    if (mCostVals.empty()) {
        return 0;
    }
    return fmaxf(
        0, mCostVals.back().first * VAL + mScrollMargin * 2 - mBounds.w());
}

float UpgradeProgressBar::toValue(const Number& val) {
    return (val < 1 ? val : val.logTenCopy() + 1).toFloat();
}

// UpgradeDisplay
const SDL_Color UpgradeDisplay::BKGRND = GRAY;
const Rect UpgradeDisplay::RECT(0, 0, 100, 100);

UpgradeDisplay::UpgradeDisplay()
    : mPos(std::make_shared<UIComponent>(Rect(), Elevation::UPGRADES)),
      mDrag(std::make_shared<DragComponent>(-1)) {
    SDL_Point screenDim = RenderSystem::getWindowSize();
    mPos->rect = Rect(0, 0, screenDim.x, fmin(screenDim.y / 5, RECT.h()));
    mPos->rect.setPosX(screenDim.x / 2, Rect::Align::CENTER);

    mTex = TextureBuilder(mPos->rect.W(), mPos->rect.H());
    mTexData.set(mTex.getTexture());
}

void UpgradeDisplay::init() {
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
            [this](UpgradeListPtr us) { onSetUpgrades(us); },
            [this](UpgradeListPtr us, ParameterSystem::ValueParam val) {
                onSetUpgrades(us, val);
            });
}

void UpgradeDisplay::onResize(ResizeData data) {
    mPos->rect = Rect(0, 0, data.newW, fmin(data.newH / 5, RECT.h()));
    mPos->rect.setPosX(data.newW / 2, Rect::Align::CENTER);

    mTex = TextureBuilder(mPos->rect.W(), mPos->rect.H());
    mTexData.set(mTex.getTexture());
}
void UpgradeDisplay::onUpdate(Time dt) {
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
void UpgradeDisplay::onRender(SDL_Renderer* r) {
    RectShape rect(BKGRND);
    mTex.draw(rect);

    if (mUpRenderer) {
        mUpRenderer->draw(mTex, mScroll);
    }

    // Draw texture to screen
    mTexData.setDest(mPos->rect);
    TextureBuilder().draw(mTexData);
}
void UpgradeDisplay::onClick(Event::MouseButton b, bool clicked) {
    if (clicked && mUpRenderer) {
        mUpRenderer->onClick(b.clickPos);
    }
}
void UpgradeDisplay::onDrag(int mouseX, int mouseY, float mouseDx,
                            float mouseDy) {
    scroll(-mouseDx);
    mScrollV = mouseDx;
}
void UpgradeDisplay::onDragStart() { mScrollV = 0; }
void UpgradeDisplay::onHover(SDL_Point mouse) {
    mUpDescRenderSub.reset();

    if (mUpRenderer) {
        mUpDescRenderSub = mUpRenderer->onHover(
            mouse, {mouse.x - mPos->rect.X(), mouse.y - mPos->rect.Y()});
        if (mUpDescRenderSub) {
            mUpDescRenderSub->get<RenderObservable::DATA>()->mouse = false;
        }
    }
}
void UpgradeDisplay::onMouseLeave() { mUpDescRenderSub.reset(); }
void UpgradeDisplay::onSetUpgrades(UpgradeListPtr upgrades) {
    mUpRenderer =
        upgrades ? std::make_unique<UpgradeScroller>(upgrades) : nullptr;
    scroll(0);
}
void UpgradeDisplay::onSetUpgrades(UpgradeListPtr upgrades,
                                   ParameterSystem::ValueParam val) {
    mUpRenderer = upgrades ? std::make_unique<UpgradeProgressBar>(upgrades, val)
                           : nullptr;
    scroll(0);
}

void UpgradeDisplay::scroll(float dScroll) {
    float maxScroll = mUpRenderer ? mUpRenderer->maxScroll() : 0;
    float minScroll = mUpRenderer ? mUpRenderer->minScroll() : 0;
    mScroll = fmax(fmin(mScroll + dScroll, maxScroll), minScroll);
}