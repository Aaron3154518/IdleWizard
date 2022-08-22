#include "Upgrade.h"

/*
const SDL_Color Upgrade::DESC_BKGRND{175, 175, 175, 255};
const FontData Upgrade::DESC_FONT{-1, 20, "|"};

// Defaults
const ParameterSystem::BaseValue Upgrade::Defaults::CRYSTAL_MAGIC =
    ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic);
const ParameterSystem::BaseValue Upgrade::Defaults::CRYSTAL_SHARDS =
    ParameterSystem::Param<CRYSTAL>(CrystalParams::Shards);
const ParameterSystem::BaseValue Upgrade::Defaults::CATALYST_MAGIC =
    ParameterSystem::Param<CATALYST>(CatalystParams::Magic);

bool Upgrade::Defaults::CanBuy(UpgradePtr u) {
    return !u->mCostSrc || !u->mMoneySrc ||
           u->mCostSrc->get() <= u->mMoneySrc->get();
}

std::string Upgrade::Defaults::AdditiveEffect(const Number& effect) {
    return "+" + effect.toString();
}
std::string Upgrade::Defaults::MultiplicativeEffect(const Number& effect) {
    return effect.toString() + "x";
}
std::string Upgrade::Defaults::PercentEffect(const Number& effect) {
    return (effect * 100).toString() + "%";
}

// Upgrade
Upgrade::Upgrade(ParameterSystem::BaseValue lvl) {
    mLevelSrc = std::make_unique<ParameterSystem::BaseValue>(lvl);
}

Upgrade& Upgrade::setMaxLevel(int maxLevel) {
    mMaxLevel = maxLevel;
    return *this;
}

Upgrade& Upgrade::setOnLevel(std::function<void(const Number&)> func) {
    mLevelSub = mLevelSrc->subscribe(func);
}

Upgrade& Upgrade::setMoney(ParameterSystem::BaseValue param) {
    mMoneySrc = std::make_unique<ParameterSystem::BaseValue>(param);
    return *this;
}

Upgrade& Upgrade::setCost(ParameterSystem::NodeValue param,
                          std::function<Number(const Number&)> func) {
    mCostSrc = std::make_unique<ParameterSystem::NodeValue>(param);
    mCostSub = mCostSrc->subscribeTo(*mLevelSrc, func);
    return *this;
}

Upgrade& Upgrade::setEffectStr(const std::string& str) {
    mEffect = str;
    mUpdateInfo = true;
}
Upgrade& Upgrade::setEffect(
    ValueUpdate val,
    std::function<std::string(const Number&)> getEffectString) {
    clearEffects();
    addEffect(val);
    setEffectStr(getEffectString(val.node.get()));
    mEffectSub = val.node.subscribe([this, getEffectString](const Number& val) {
        setEffectStr(getEffectString(val));
    });
    return *this;
}
Upgrade& Upgrade::setEffect(StateUpdate state,
                            std::function<std::string(bool)> getEffectString) {
    clearEffects();
    addEffect(state);
    setEffectStr(getEffectString(state.node.get()));
    mEffectSub = state.node.subscribe([this, getEffectString](bool state) {
        setEffectStr(getEffectString(state));
    });
    return *this;
}
Upgrade& Upgrade::setEffects(const std::initializer_list<ValueUpdate>& values,
                             const std::initializer_list<StateUpdate>& states,
                             std::function<std::string()> getEffectString) {
    setEffectStr(getEffectString());
    auto func = [this, getEffectString]() { setEffectStr(getEffectString()); };

    clearEffects();
    for (auto val : values) {
        addEffect(val);
        if (!mEffectSub) {
            mEffectSub = val.node->subscribe(func);
        }
    }
    for (auto state : states) {
        addEffect(state);
        if (!mEffectSub) {
            mEffectSub = state.node->subscribe(func);
        }
    }
    return *this;
}

Upgrade& Upgrade::addEffect(ValueUpdate val) {
    mValueEffectSrcs.push_back(val.node);
    mEffectSubs.push_back(val.node.subscribeTo(*mLevelSrc, val.func));
    if (mEffectSub) {
        val.node->subscribe(mEffectSub);
    }
}
Upgrade& Upgrade::addEffect(StateUpdate state) {
    mStateEffectSrcs.push_back(state.node);
    mEffectSubs.push_back(state.node.subscribeTo(*mLevelSrc, state.func));
    if (mEffectSub) {
        state.node->subscribe(mEffectSub);
    }
}

Upgrade& Upgrade::clearEffects() {
    mEffectSubs.clear();
    mValueEffectSrcs.clear();
    mStateEffectSrcs.clear();
    mEffectSub.reset();
    return *this;
}

Upgrade& Upgrade::setImg(std::string img) {
    mImg = AssetManager::getTexture(img);
    return *this;
}

Upgrade& Upgrade::setDescription(std::string desc) {
    mDesc = createDescription(desc);
    return *this;
}

Upgrade::Status Upgrade::getStatus() const {
    if (mMaxLevel == 0) {
        return NOT_BUYABLE;
    }
    if (mMaxLevel > 0 && mLevelSrc->get() >= mMaxLevel) {
        return BOUGHT;
    }
    if (mCostSrc && mMoneySrc && mCostSrc->get() > mMoneySrc->get()) {
        return CANT_BUY;
    }
    return BUYABLE;
}

void Upgrade::buy() {
    if (getStatus() != BUYABLE) {
        return;
    }

    if (mCostSrc && mMoneySrc) {
        mMoneySrc->set(mMoneySrc->get() - mCostSrc->get());
    }
    mLevelSrc->set(mLevelSrc->get() + 1);
}

void Upgrade::drawIcon(TextureBuilder& tex, const Rect& r) {
    RenderData rData;
    rData.texture = mImg;
    rData.dest = r;
    rData.shrinkToTexture();
    tex.draw(rData);
}

void Upgrade::drawDescription(TextureBuilder tex, SDL_FPoint offset) {
    if (mUpdateInfo) {
        mInfo = createDescription(getInfo());
        mUpdateInfo = false;
    }

    RectData rd;
    rd.color = DESC_BKGRND;

    RenderData descData;
    if (mDesc) {
        descData.texture = mDesc;
        descData.shrinkToTexture();
        descData.dest.setPos(offset.x, offset.y, Rect::CENTER, Rect::TOP_LEFT);
    } else {
        descData.dest = Rect(offset.x, offset.y, 0, 0);
    }

    if (mIncludeInfo && mInfo) {
        RenderData infoData;
        infoData.texture = mInfo;
        infoData.shrinkToTexture();
        float w = infoData.dest.w();
        w = std::max(w, descData.dest.w());
        infoData.dest.setPos(descData.dest.cX(), descData.dest.y2(),
                             Rect::CENTER, Rect::TOP_LEFT);

        Rect rdRect(0, offset.y, w, infoData.dest.y2() - offset.y);
        rdRect.setPosX(offset.x, Rect::CENTER);
        tex.draw(rd.set(rdRect));
        tex.draw(infoData);

        if (mDesc) {
            tex.draw(descData);
        }

        rd.color = BLACK;
        tex.draw(rd.set(rd.r2, 1));
    } else if (mDesc) {
        tex.draw(rd.set(descData.dest));

        tex.draw(descData);

        rd.color = BLACK;
        tex.draw(rd.set(rd.r2, 1));
    }
}

std::string Upgrade::getInfo() const {
    std::stringstream ss;
    ss << mEffect;
    if (mMaxLevel > 0) {
        Number lvl = mLevelSrc->get();
        if (!mEffect.empty()) {
            ss << "\n";
        }
        if (mMaxLevel > 1) {
            ss << lvl << "/" << mMaxLevel << ": ";
        }
        if (lvl < mMaxLevel) {
            if (mCostSrc) {
                ss << "$" << mCostSrc->get();
            }
        } else {
            ss << (mMaxLevel > 1 ? "Maxed" : "Bought");
        }
    }
    return ss.str();
}

SharedTexture Upgrade::createDescription(std::string text) {
    if (text.empty()) {
        return nullptr;
    }

    TextData tData;
    tData.font = AssetManager::getFont(DESC_FONT);
    tData.w = RenderSystem::getWindowSize().x / 3;
    tData.text = text;
    return tData.renderTextWrapped();
}

// TileUpgrade
Upgrade& TileUpgrade::setEffect(
    ParameterSystem::NodeValue val,
    std::function<std::string(const Number&)> getEffectString) {
    clearEffects();
    mValueEffectSrcs.push_back(val);
    setEffectStr(getEffectString(val.get()));
    mEffectSub = val.subscribe([this, getEffectString](const Number& val) {
        setEffectStr(getEffectString(val));
    });
    return *this;
}
Upgrade& TileUpgrade::setEffect(
    ParameterSystem::NodeState state,
    std::function<std::string(bool)> getEffectString) {
    clearEffects();
    mStateEffectSrcs.push_back(state);
    setEffectStr(getEffectString(state.get()));
    mEffectSub = state.subscribe([this, getEffectString](bool state) {
        setEffectStr(getEffectString(state));
    });
    return *this;
}
Upgrade& TileUpgrade::setEffects(
    const std::initializer_list<ParameterSystem::NodeValue>& values,
    const std::initializer_list<ParameterSystem::NodeState>& states,
    std::function<std::string()> getEffectString) {
    setEffectStr(getEffectString());
    auto func = [this, getEffectString]() { setEffectStr(getEffectString()); };

    clearEffects();
    for (auto val : values) {
        mValueEffectSrcs.push_back(val);
        if (!mEffectSub) {
            mEffectSub = val->subscribe(func);
        }
    }
    for (auto state : states) {
        mStateEffectSrcs.push_back(state);
        if (!mEffectSub) {
            mEffectSub = state->subscribe(func);
        }
    }
    return *this;
}
*/

// UpgradeList
int UpgradeList::size() const { return mSubscriptions.size(); }

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
    }

    auto drawUpgrade = [this, &tex, offset](Rect r, SubscriptionPtr sub) {
        r.move(offset.x, offset.y);
        RectData rd;
        if (!sub) {
            rd.color = WHITE;
            tex.draw(rd.set(r, 3));
        } else {
            auto up = sub->get<DATA>();
            switch (up->getStatus()) {
                case Upgrade::Status::BOUGHT:
                    rd.color = BLUE;
                    break;
                case Upgrade::Status::CAN_BUY:
                    rd.color = GREEN;
                    break;
                case Upgrade::Status::CANT_BUY:
                    rd.color = RED;
                    break;
                case Upgrade::Status::NOT_BUYABLE:
                    rd.color = BLACK;
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

    std::vector<float> rectAngles(mSubscriptions.size());
    int backLen = 0, frontLen = 0;

    for (int i = NUM_STEPS; i >= 0; i--) {
        int sign = 1;
        do {
            float angle = fmod(minTheta + i * sign * STEP + TWO_PI, TWO_PI);
            int idx = baseIdx - (NUM_STEPS - i) * sign;
            if (idx >= 0 && idx < mSubscriptions.size()) {
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
    int i = 0, bIdx = 0, fIdx = 0;
    for (auto sub : *this) {
        float angle = rectAngles[i++];
        (angle < M_PI ? mBackRects.at(bIdx++) : mFrontRects.at(fIdx++)) =
            std::make_pair(getRect(angle), sub);
    }
}

UpgradeBasePtr UpgradeList::Get(SubscriptionPtr sub) {
    return sub->get<UpgradeList::DATA>();
}

// UpgradeScroller
const SDL_Color UpgradeScroller::BGKRND = GRAY;
const Rect UpgradeScroller::RECT(0, 0, 100, 100);

UpgradeScroller::UpgradeScroller()
    : mPos(std::make_shared<UIComponent>(Rect(), Elevation::UPGRADES)),
      mDrag(std::make_shared<DragComponent>(-1)) {
    SDL_Point screenDim = RenderSystem::getWindowSize();
    mPos->rect = Rect(0, 0, screenDim.x, fmin(screenDim.y / 5, RECT.h()));
    mPos->rect.setPosX(screenDim.x / 2, Rect::Align::CENTER);

    mTex = TextureBuilder(mPos->rect.W(), mPos->rect.H());
    mTexData.texture = mTex.getTexture();
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
    mTexData.texture = mTex.getTexture();
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
    RectData rd;
    rd.color = BGKRND;
    mTex.draw(rd);

    if (mUpgrades) {
        mUpgrades->draw(mTex, mScroll);
    }

    // Draw texture to screen
    mTexData.dest = mPos->rect;
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
