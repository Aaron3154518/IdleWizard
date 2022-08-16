#include "Upgrade.h"

// Upgrade
const SDL_Color Upgrade::DESC_BKGRND{175, 175, 175, 255};
const FontData Upgrade::DESC_FONT{-1, 20, "|"};

const ParameterSystem::ParamBase Upgrade::ParamSources::NONE;
const ParameterSystem::Param<CRYSTAL> Upgrade::ParamSources::CRYSTAL_MAGIC =
    ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic);

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

int Upgrade::getMaxLevel() const { return mMaxLevel; }
int Upgrade::getLevel() const { return mLevel; }
const std::string& Upgrade::getEffect() const { return mEffect; }
bool Upgrade::hasEffectSrc() const { return (bool)mEffectSrc; }
const ParameterSystem::ParamBase& Upgrade::getEffectSrc() const {
    return hasEffectSrc() ? *mEffectSrc : ParamSources::NONE;
}
bool Upgrade::hasCostSrc() const { return (bool)mCostSrc; }
const ParameterSystem::ParamBase& Upgrade::getCostSrc() const {
    return hasCostSrc() ? *mCostSrc : ParamSources::NONE;
}
bool Upgrade::hasMoneySrc() const { return (bool)mMoneySrc; }
const ParameterSystem::ParamBase& Upgrade::getMoneySrc() const {
    return hasMoneySrc() ? *mMoneySrc : ParamSources::NONE;
}

Upgrade& Upgrade::setMaxLevel(int maxLevel) {
    mMaxLevel = maxLevel;
    return *this;
}
Upgrade& Upgrade::setLevel(int level) {
    mLevel = level;
    return *this;
}
Upgrade& Upgrade::setEffect(const std::string& effect) {
    mEffect = effect;
    return *this;
}
Upgrade& Upgrade::clearEffectSource() {
    mEffectSrc.reset();
    mEffectSub.reset();
    return *this;
}
Upgrade& Upgrade::clearCostSource() {
    mCostSrc.reset();
    mCostSub.reset();
    return *this;
}
Upgrade& Upgrade::clearMoneySource() {
    mMoneySrc.reset();
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

void Upgrade::drawDescription(TextureBuilder tex, SDL_FPoint offset) const {
    RectData rd;
    rd.color = DESC_BKGRND;

    RenderData descData;
    if (mDesc) {
        descData.texture = mDesc;
        descData.fitToTexture();
        descData.dest.setPos(offset.x, offset.y, Rect::CENTER, Rect::TOP_LEFT);
    } else {
        descData.dest = Rect(offset.x, offset.y, 0, 0);
    }

    if (mIncludeInfo && mInfo) {
        RenderData infoData;
        infoData.texture = mInfo;
        infoData.fitToTexture();
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
        if (!mEffect.empty()) {
            ss << "\n";
        }
        if (mMaxLevel > 1) {
            ss << mLevel << "/" << mMaxLevel << ": ";
        }
        if (mLevel < mMaxLevel) {
            if (mCostSrc) {
                ss << "$" << mCostSrc->get();
            }
        } else {
            ss << (mMaxLevel > 1 ? "Maxed" : "Bought");
        }
    }
    return ss.str();
}

void Upgrade::updateEffect() {
    if (mEffectSub) {
        mEffectSub->get<0>()();
    }
}
void Upgrade::updateInfo() { mInfo = createDescription(getInfo()); }

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

// UpgradeList
UpgradeList::SubscriptionPtr UpgradeList::subscribe(
    std::function<void(UpgradePtr)> func, UpgradePtr up) {
    return UpgradeListBase::subscribe(func, Upgrade::Defaults::CanBuy, up);
}
UpgradeList::SubscriptionPtr UpgradeList::subscribe(UpgradePtr up) {
    return UpgradeListBase::subscribe([](UpgradePtr u) {},
                                      Upgrade::Defaults::CanBuy, up);
}

void UpgradeList::onSubscribe(SubscriptionPtr sub) {
    sub->get<ON_LEVEL>()(sub->get<DATA>());
    sub->get<DATA>()->updateInfo();
}

UpgradeList::UpgradeStatus UpgradeList::getSubStatus(SubscriptionPtr sub) {
    UpgradePtr up = sub->get<DATA>();
    if (up->getMaxLevel() == 0) {
        return UpgradeStatus::NOT_BUYABLE;
    }
    if (up->getMaxLevel() > 0 && up->getLevel() >= up->getMaxLevel()) {
        return UpgradeStatus::BOUGHT;
    }
    return sub->get<CAN_BUY>()(up) ? UpgradeStatus::BUYABLE
                                   : UpgradeStatus::CANT_BUY;
}
void UpgradeList::onSubClick(SubscriptionPtr sub) {
    UpgradePtr up = sub->get<DATA>();
    if ((up->getMaxLevel() < 0 || up->getLevel() < up->getMaxLevel()) &&
        getSubStatus(sub) == UpgradeStatus::BUYABLE) {
        if (up->hasMoneySrc() && up->hasCostSrc()) {
            up->getMoneySrc().set(up->getMoneySrc().get() -
                                  up->getCostSrc().get());
        }
        up->setLevel(up->getLevel() + 1);
        sub->get<ON_LEVEL>()(up);
        up->updateInfo();
    }
}

int UpgradeList::size() const { return mSubscriptions.size(); }

void UpgradeList::onClick(SDL_Point mouse) {
    for (auto pair : mFrontRects) {
        if (SDL_PointInRect(&mouse, pair.first)) {
            onSubClick(pair.second.lock());
            return;
        }
    }
    for (auto pair : mBackRects) {
        if (SDL_PointInRect(&mouse, pair.first)) {
            onSubClick(pair.second.lock());
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
        } else {
            switch (getSubStatus(sub)) {
                case UpgradeStatus::BOUGHT:
                    rd.color = BLUE;
                    break;
                case UpgradeStatus::BUYABLE:
                    rd.color = GREEN;
                    break;
                case UpgradeStatus::CANT_BUY:
                    rd.color = RED;
                    break;
                case UpgradeStatus::NOT_BUYABLE:
                    rd.color = BLACK;
                    break;
            }
        }
        tex.draw(rd.set(r, 3));

        if (sub) {
            RenderData rData;
            rData.texture = sub->get<DATA>()->mImg;
            rData.dest = r;
            rData.fitToTexture();
            tex.draw(rData);
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

UpgradePtr UpgradeList::Get(SubscriptionPtr sub) {
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
        ServiceSystem::Get<UpgradeService, UpgradeListObservable>()->subscribe(
            std::bind(&UpgradeScroller::onSetUpgrades, this,
                      std::placeholders::_1));
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
