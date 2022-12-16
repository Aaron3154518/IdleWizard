#include "UpgradeList.h"

// UpgradeList
int UpgradeList::size() const { return getNumActive(); }

float UpgradeList::getScroll() const { return mScroll; }
void UpgradeList::setScroll(float scroll) { mScroll = scroll; }
bool UpgradeList::isOpen() const { return mOpen; }
void UpgradeList::setOpen(bool open) { mOpen = open; }

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
UpgradeRenderer::UpgradeRenderer(UpgradeListPtr upgrades)
    : mUpgrades(upgrades) {}

void UpgradeRenderer::onClick(SDL_Point mouse) {}
RenderObservable::SubscriptionPtr UpgradeRenderer::onHover(SDL_Point mouse,
                                                           SDL_Point relMouse) {
    return nullptr;
}

void UpgradeRenderer::draw(TextureBuilder tex) {}
void UpgradeRenderer::update(Rect pos, float scroll) {}

float UpgradeRenderer::minScroll() const { return mMinScroll; }
float UpgradeRenderer::maxScroll() const { return mMaxScroll; }

void UpgradeRenderer::open(Rect pos, float scroll) {
    mUpgrades->setOpen(true);
    update(pos, scroll);
}
void UpgradeRenderer::close(float scroll) {
    mUpgrades->setOpen(false);
    mUpgrades->setScroll(scroll);
}

float UpgradeRenderer::getScroll() const { return mUpgrades->getScroll(); }

// UpgradeScroller
UpgradeScroller::UpgradeScroller(UpgradeListPtr upgrades, RenderTextureCPtr img)
    : UpgradeRenderer(upgrades) {
    mImg.set(img);
}

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

void UpgradeScroller::draw(TextureBuilder tex) {
    auto drawUpgrade = [this, &tex](Rect r, UpgradeList::SubscriptionPtr sub) {
        RectShape rd;
        rd.mColor = GRAY;
        tex.draw(rd.set(r));
        if (sub) {
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

    tex.draw(mImg);

    for (auto pair : mFrontRects) {
        drawUpgrade(pair.first, pair.second.lock());
    }
}

void UpgradeScroller::update(Rect pos, float scroll) {
    mUpgrades->prune();

    mMaxScroll = pos.empty()
                     ? 0
                     : pos.w() * (mUpgrades ? mUpgrades->size() - 1 : 0) /
                           floor(pos.w() * 2 / pos.h());

    const float w = pos.h() / 2;
    float a = (pos.w() - w) / 2;
    float b = (pos.h() - w) / 2;

    Rect imgR(0, 0, w * 2, w * 2);
    imgR.setPos(pos.cX(), pos.cY(), Rect::Align::CENTER);
    mImg.setDest(imgR);

    const float TWO_PI = 2 * M_PI;
    const float HALF_PI = M_PI / 2;
    const int NUM_STEPS = floor(pos.w() / w);
    const float STEP = M_PI / NUM_STEPS;
    const float ERR = 1e-5;

    // Compute total scroll angle
    float scrollAngle = scroll * TWO_PI / (pos.w() * 2);
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

    float cX = pos.w() / 2;
    float cY = pos.h() / 2 - w / 4;

    int num = mUpgrades->getNumActive();
    std::vector<float> rectAngles(num);
    int backLen = 0, frontLen = 0;

    for (int i = NUM_STEPS; i >= 0; i--) {
        int sign = 1;
        do {
            float angle = fmod(minTheta + i * sign * STEP + TWO_PI, TWO_PI);
            int idx = baseIdx - (NUM_STEPS - i) * sign;
            if (idx >= 0 && idx < num) {
                rectAngles.at(idx) = angle;
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
        float angle = rectAngles.at(i);
        bool back = angle < M_PI;
        mIdxMap.at(i) = std::make_pair(!back, back ? bIdx : fIdx);
        (back ? mBackRects.at(bIdx++) : mFrontRects.at(fIdx++)) =
            std::make_pair(getRect(angle), sub);
        i++;
    }
}

// UpgradeProgressBar
const int UpgradeProgressBar::BUCKET_W = 100;

UpgradeProgressBar::UpgradeProgressBar(UpgradeListPtr upgrades,
                                       ParameterSystem::BaseValue money,
                                       ParameterSystem::ValueParam val)
    : UpgradeRenderer(upgrades), mMoneyParam(money), mValParam(val) {
    mPb.set(BLUE, {200, 200, 200, 255});
}

RenderObservable::SubscriptionPtr UpgradeProgressBar::onHover(
    SDL_Point mouse, SDL_Point relMouse) {
    for (auto pair : mRects) {
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
    if (SDL_PointInRect(&relMouse, mPb.dest)) {
        SDL_FPoint pos{(float)mouse.x, mPb.dest.y2() + (mouse.y - relMouse.y)};
        // Construct description string
        std::stringstream ss;
        ss << "Current: " << mValParam.get() << "{i}\nNext: " << mNextCost
           << "{i}\nUnlocked: " << mUnlocked << "/" << mUpgrades->size();
        std::string desc = ss.str();
        std::vector<RenderTextureCPtr> imgs = {
            Money::GetMoneyIcon(mMoneyParam), Money::GetMoneyIcon(mMoneyParam)};
        return ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [pos, desc, imgs](SDL_Renderer* r) {
                Display u;
                u.setDescription({desc, imgs});
                u.drawDescription(TextureBuilder(), pos);
            },
            std::make_shared<UIComponent>(Rect(), Elevation::OVERLAYS));
    }
    return nullptr;
}

void UpgradeProgressBar::draw(TextureBuilder tex) {
    for (auto pair : mRects) {
        auto up = pair.second.lock();
        if (up) {
            up->get<UpgradeList::DATA>()->drawIcon(tex, pair.first,
                                                   mRs.get().r1);
        }
    }

    tex.draw(mPb);
    tex.draw(mRs);
}

void UpgradeProgressBar::update(Rect pos, float scroll) {
    mRects.clear();

    // Get upgrades with log costs
    std::vector<UpgradeCost> upCosts;
    for (auto sub : *mUpgrades) {
        auto& cost = sub->get<UpgradeList::DATA>()->getCost();
        if (cost) {
            upCosts.push_back(std::make_pair(cost->getCost(), sub));
        }
    }

    // Sort by increasing cost
    std::stable_sort(upCosts.begin(), upCosts.end(),
                     [](const UpgradeCost& lhs, const UpgradeCost& rhs) {
                         return lhs.first < rhs.first;
                     });

    if (upCosts.empty()) {
        return;
    }

    // Collect data on unlocked
    Number currVal = mValParam.get();
    mNextCost = upCosts.back().first;
    for (mUnlocked = 0; mUnlocked < upCosts.size(); mUnlocked++) {
        if (currVal < upCosts.at(mUnlocked).first) {
            mNextCost = upCosts.at(mUnlocked).first;
            break;
        }
    }

    int marginX = Upgrade::GetDescWidth() / 2;
    int marginY = pos.h() / 10;
    Rect bounds(marginX, marginY, pos.w() - marginX * 2, pos.h() - marginY * 2);
    float pbH = bounds.h() / 5;
    float upW = (bounds.h() - pbH) / 2;
    float scrollMargin = upW / 2;

    float maxCost = toValue(upCosts.back().first);

    mMaxScroll =
        upCosts.empty()
            ? 0
            : fmaxf(0, maxCost * BUCKET_W + scrollMargin * 2 - bounds.w());

    scroll -= scrollMargin;
    float start = scroll / BUCKET_W;
    float val_start = fmaxf(start, 0);
    float end = start + bounds.w() / BUCKET_W;
    float val_end = fminf(end, maxCost);
    float amnt = toValue(mValParam.get());

    mRs.set(bounds, 2);
    mPb.set(Rect(bounds.x() + BUCKET_W * (val_start - start), bounds.y() + upW,
                 BUCKET_W * (val_end - val_start), pbH))
        .set(amnt - val_start, val_end - val_start);

    for (int i = 0; i < upCosts.size(); i++) {
        auto& pair = upCosts.at(i);
        float val = toValue(pair.first);
        if (val >= end + scrollMargin) {
            break;
        }
        if (val > start - scrollMargin) {
            auto upPtr = pair.second.lock();
            if (upPtr) {
                Rect r(0, 0, upW, upW);
                r.setPos(bounds.x() + BUCKET_W * (val - start),
                         i % 2 == 0 ? bounds.y() : bounds.y2() - r.h(),
                         Rect::Align::CENTER, Rect::Align::TOP_LEFT);
                mRects.push_back(std::make_pair(r, pair.second));
            }
        }
    }
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
            [this](UpgradeListPtr us, RenderTextureCPtr img) {
                onSetUpgrades(us, img);
            },
            [this](UpgradeListPtr us, ParameterSystem::BaseValue money,
                   ParameterSystem::ValueParam val) {
                onSetUpgrades(us, money, val);
            });
}

void UpgradeDisplay::onResize(ResizeData data) {
    mPos->rect = Rect(0, 0, data.newW, fmin(data.newH / 5, RECT.h()));
    mPos->rect.setPosX(data.newW / 2, Rect::Align::CENTER);

    mTex = TextureBuilder(mPos->rect.W(), mPos->rect.H());
    mTexData.set(mTex.getTexture());

    if (mUpRenderer) {
        mUpRenderer->update(mPos->rect, mScroll);
    }
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
        mUpRenderer->draw(mTex);
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
bool UpgradeDisplay::onTimer(Time& timer) {
    if (mUpRenderer) {
        mUpRenderer->update(mPos->rect, mScroll);
    }
    return true;
}
void UpgradeDisplay::setUpgrades(UpgradeRendererPtr upRenderer) {
    if (mUpRenderer) {
        mUpRenderer->close(mScroll);
    }
    mUpRenderer = std::move(upRenderer);
    if (mUpRenderer) {
        mUpRenderer->open(mPos->rect, mUpRenderer->minScroll());
        scroll(mUpRenderer->getScroll() - mScroll);
    }
}
void UpgradeDisplay::onSetUpgrades(UpgradeListPtr upgrades,
                                   RenderTextureCPtr img) {
    setUpgrades(upgrades ? std::make_unique<UpgradeScroller>(upgrades, img)
                         : nullptr);
}
void UpgradeDisplay::onSetUpgrades(UpgradeListPtr upgrades,
                                   ParameterSystem::BaseValue money,
                                   ParameterSystem::ValueParam val) {
    setUpgrades(upgrades
                    ? std::make_unique<UpgradeProgressBar>(upgrades, money, val)
                    : nullptr);
}

void UpgradeDisplay::scroll(float dScroll) {
    float maxScroll = mUpRenderer ? mUpRenderer->maxScroll() : 0;
    float minScroll = mUpRenderer ? mUpRenderer->minScroll() : 0;
    mScroll = fmax(fmin(mScroll + dScroll, maxScroll), minScroll);
    if (mUpRenderer) {
        mUpRenderer->update(mPos->rect, mScroll);
    }
}