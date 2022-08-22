#include "Empty.h"

// UpgradeBase
const SDL_Color UpgradeBase::DESC_BKGRND{175, 175, 175, 255};
const FontData UpgradeBase::DESC_FONT{-1, 20, "|"};

const ParameterSystem::BaseValue UpgradeBase::Defaults::CRYSTAL_MAGIC =
    ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic);
const ParameterSystem::BaseValue UpgradeBase::Defaults::CRYSTAL_SHARDS =
    ParameterSystem::Param<CRYSTAL>(CrystalParams::Shards);

std::string Upgrade::Defaults::AdditiveEffect(const Number& effect) {
    return "+" + effect.toString();
}
std::string Upgrade::Defaults::MultiplicativeEffect(const Number& effect) {
    return effect.toString() + "x";
}
std::string Upgrade::Defaults::PercentEffect(const Number& effect) {
    return (effect * 100).toString() + "%";
}

UpgradeBase::Status UpgradeBase::getStatus() { return NOT_BUYABLE; }

void UpgradeBase::buy() {}

void UpgradeBase::setImage(const std::string& file) {
    mImg = AssetManager::getTexture(file);
}

void UpgradeBase::setDescription(const std::string& desc) {
    mDesc = createDescription(desc);
}

void UpgradeBase::setInfo(const std::string& info) {
    mInfoStr = info;
    mInfo = createDescription(info);
}

void UpgradeBase::drawIcon(TextureBuilder& tex, const Rect& r) {
    if (!mImg) {
        return;
    }

    RenderData rData;
    rData.texture = mImg;
    rData.dest = r;
    rData.shrinkToTexture();
    tex.draw(rData);
}

void UpgradeBase::drawDescription(TextureBuilder tex, SDL_FPoint offset) {
    if (mUpdateInfo) {
        setInfo(mInfoStr);
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

    if (mInfo) {
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

SharedTexture UpgradeBase::createDescription(std::string text) {
    if (text.empty()) {
        return nullptr;
    }

    TextData tData;
    tData.font = AssetManager::getFont(DESC_FONT);
    tData.w = RenderSystem::getWindowSize().x / 3;
    tData.text = text;
    return tData.renderTextWrapped();
}

// Display
UpgradeBase::Status Display::getStatus() { return NOT_BUYABLE; }

void Display::setEffect(ParameterSystem::ValueParam param,
                        std::function<std::string(const Number&)> func) {
    setEffects({param}, {}, [param, func]() { return func(param.get()); });
}
void Display::setEffect(ParameterSystem::StateParam param,
                        std::function<std::string(bool)> func) {
    setEffects({}, {param}, [param, func]() { return func(param.get()); });
}
void Display::setEffects(
    const std::initializer_list<ParameterSystem::ValueParam>& valueParams,
    const std::initializer_list<ParameterSystem::StateParam>& stateParams,
    std::function<std::string()> func) {
    mEffectSub =
        ParameterSystem::subscribe(valueParams, stateParams, [this, func]() {
            mInfoStr = func();
            mUpdateInfo = true;
        });
}

// Toggle
Toggle::Toggle(LevelFunc onLevel, unsigned int numStates)
    : mOnLevel(onLevel), mNumStates(numStates) {
    setLevel(0);
}

UpgradeBase::Status Toggle::getStatus() { return CAN_BUY; }

void Toggle::buy() {
    mLevel = mNumStates == 0 ? 0 : (mLevel + 1) % mNumStates;
    mOnLevel(mLevel, *this);
}

void Toggle::setLevel(unsigned int lvl) {
    mLevel = mNumStates == 0 ? 0 : lvl % mNumStates;
    mOnLevel(mLevel, *this);
}

// Cost
Upgrade::Cost::Cost(ParameterSystem::BaseValue money,
                    ParameterSystem::NodeValue cost)
    : mMoney(money), mCost(cost) {}
Upgrade::Cost::Cost(ParameterSystem::BaseValue level,
                    ParameterSystem::BaseValue money,
                    ParameterSystem::NodeValue cost,
                    std::function<Number(const Number&)> costFunc)
    : Cost(money, cost) {
    mCostSub = cost.subscribeTo(level, costFunc);
}

const Number& Upgrade::Cost::getCost() const { return mCost.get(); }
const Number& Upgrade::Cost::getMoney() const { return mMoney.get(); }
bool Upgrade::Cost::canBuy() const { return mCost.get() <= mMoney.get(); }
void Upgrade::Cost::buy() const { mMoney.set(mMoney.get() - mCost.get()); }

ParameterSystem::ParameterSubscriptionPtr Upgrade::Cost::subscribe(
    std::function<void()> func) const {
    ParameterSystem::ParameterSubscriptionPtr sub = mCost.subscribe(func);
    mMoney.getObservable()->subscribe(sub);
    return sub;
}

// Effects
Upgrade::Effects::Effects() {}
Upgrade::Effects::Effects(EffectFunc func) : mGetEffect(func) {}

Upgrade::Effects& Upgrade::Effects::addEffect(ParameterSystem::NodeValue param,
                                              ValueFunc func) {
    mValueParams.push_back(std::make_pair(param, func));
    return *this;
}
Upgrade::Effects& Upgrade::Effects::addEffect(
    ParameterSystem::NodeValue param, ValueFunc valFunc,
    std::function<std::string(const Number&)> effFunc) {
    addEffect(param, valFunc);
    mGetEffect = [param, effFunc]() { return effFunc(param.get()); };
    return *this;
}
Upgrade::Effects& Upgrade::Effects::addEffect(ParameterSystem::NodeState param,
                                              StateFunc func) {
    mStateParams.push_back(std::make_pair(param, func));
    return *this;
}
Upgrade::Effects& Upgrade::Effects::addEffect(
    ParameterSystem::NodeState param, StateFunc stateFunc,
    std::function<std::string(bool)> effFunc) {
    addEffect(param, stateFunc);
    mGetEffect = [param, effFunc]() { return effFunc(param.get()); };
    return *this;
}

std::list<ParameterSystem::ParameterSubscriptionPtr>
Upgrade::Effects::subscribeToLevel(ParameterSystem::BaseValue level) const {
    std::list<ParameterSystem::ParameterSubscriptionPtr> subs;
    for (auto param : mValueParams) {
        subs.push_back(param.first.subscribeTo(level, param.second));
    }
    for (auto param : mStateParams) {
        subs.push_back(param.first.subscribeTo(level, param.second));
    }
    return subs;
}
ParameterSystem::ParameterSubscriptionPtr Upgrade::Effects::subscribeToEffects(
    std::function<void(const std::string&)> func) const {
    if (!mGetEffect) {
        return nullptr;
    }

    auto getEffect = mGetEffect;
    auto useEffect = [func, getEffect]() { func(getEffect()); };

    ParameterSystem::ParameterSubscriptionPtr sub;
    for (auto param : mValueParams) {
        if (!sub) {
            sub = param.first.subscribe(useEffect);
        } else {
            param.first.getObservable()->subscribe(sub);
        }
    }
    for (auto param : mStateParams) {
        if (!sub) {
            sub = param.first.subscribe(useEffect);
        } else {
            param.first.getObservable()->subscribe(sub);
        }
    }
    return sub;
}

// Upgrade
Upgrade::Upgrade(ParameterSystem::BaseValue level, unsigned int maxLevel,
                 std::function<void(const Number&)> onLevel)
    : mLevel(level), mMaxLevel(maxLevel) {
    mLevelSub = level.subscribe([this, onLevel]() {
        onLevel(mLevel.get());
        updateInfo();
    });
}
Upgrade::Upgrade(ParameterSystem::BaseValue level,
                 ParameterSystem::ValueParam maxLevel,
                 std::function<void(const Number&)> onLevel)
    : Upgrade(level, std::max(0, maxLevel.get().toInt()), onLevel) {
    mMaxLevelSub = maxLevel.subscribe(
        [this](const Number& lvl) { mMaxLevel = std::max(0, lvl.toInt()); });
}

Upgrade::Status Upgrade::getStatus() {
    if (mMaxLevel == 0) {
        return NOT_BUYABLE;
    }
    if (mLevel.get() >= mMaxLevel) {
        return BOUGHT;
    }
    return mCost && !mCost->canBuy() ? CANT_BUY : CAN_BUY;
}
void Upgrade::buy() {
    if (getStatus() != CAN_BUY) {
        return;
    }

    if (mCost) {
        mCost->buy();
    }
    mLevel.set(mLevel.get() + 1);
}

void Upgrade::setCost(ParameterSystem::BaseValue money,
                      ParameterSystem::NodeValue cost) {
    mCost = std::make_unique<Cost>(money, cost);
}
void Upgrade::setCost(ParameterSystem::BaseValue money,
                      ParameterSystem::NodeValue cost,
                      std::function<Number(const Number&)> costFunc) {
    mCost = std::make_unique<Cost>(mLevel, money, cost, costFunc);
    mCostSub = mCost->subscribe([this]() { updateInfo(); });
}
void Upgrade::clearCost() { mCost.reset(); }

void Upgrade::setEffects(const Effects& effects) {
    mEffectLevelSubs = effects.subscribeToLevel(mLevel);
    mEffectSub = effects.subscribeToEffects([this](const std::string& str) {
        mEffectStr = str;
        updateInfo();
    });
}
void Upgrade::clearEffects() {
    mEffectLevelSubs.clear();
    mEffectSub.reset();
}

void Upgrade::updateInfo() {
    std::stringstream ss;
    ss << mEffectStr;
    if (mMaxLevel > 0) {
        Number lvl = mLevel.get();
        if (!mEffectStr.empty()) {
            ss << "\n";
        }
        if (mMaxLevel > 1) {
            ss << lvl << "/" << mMaxLevel << ": ";
        }
        if (lvl < mMaxLevel) {
            if (mCost) {
                ss << "$" << mCost->getCost();
            }
        } else {
            ss << (mMaxLevel > 1 ? "Maxed" : "Bought");
        }
    }
    mInfoStr = ss.str();
    mUpdateInfo = true;
}