#include "Upgrade.h"

// UpgradeBase
const SDL_Color UpgradeBase::DESC_BKGRND{175, 175, 175, 255};
const FontData UpgradeBase::DESC_FONT{-1, 22, "|"};

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

void UpgradeBase::setImage(WizardId id) {
    mWizImgSub = WizardSystem::GetWizardImageObservable()->subscribe(
        [this](const RenderData& data) { mImg = data; }, id);
}

void UpgradeBase::setImage(const std::string& file) {
    mWizImgSub.reset();
    mImg.set(file);
}

void UpgradeBase::setDescription(const std::string& desc) {
    mDesc = createDescription(desc);
}

void UpgradeBase::setInfo(const std::string& info) {
    mInfoStr = info;
    mUpdateInfo = true;
}

void UpgradeBase::drawIcon(TextureBuilder& tex, const Rect& r) {
    tex.draw(mImg.setDest(r));
}

void UpgradeBase::drawDescription(TextureBuilder tex, SDL_FPoint offset) {
    if (mUpdateInfo) {
        mInfo = createDescription(mInfoStr);
        mUpdateInfo = false;
    }

    RectShape rd = RectShape(DESC_BKGRND);

    RenderData descData = RenderData()
                              .setDest(Rect(offset.x, offset.y, 0, 0))
                              .setFit(RenderData::FitMode::Texture);
    if (mDesc) {
        descData.set(mDesc).setFitAlign(Rect::CENTER, Rect::TOP_LEFT);
    }

    if (mInfo) {
        RenderData infoData = RenderData()
                                  .set(mInfo)
                                  .setFitAlign(Rect::CENTER, Rect::TOP_LEFT)
                                  .setDest(Rect(0, 0, 0, 0));
        float w = infoData.getDest().w();
        w = std::max(w, descData.getDest().w());
        infoData.setDest(
            Rect(descData.getDest().cX(), descData.getDest().y2(), 0, 0));

        Rect rdRect(0, offset.y, w, infoData.getDest().y2() - offset.y);
        rdRect.setPosX(offset.x, Rect::CENTER);
        tex.draw(rd.set(rdRect));
        tex.draw(infoData);

        if (mDesc) {
            tex.draw(descData);
        }

        rd.mColor = BLACK;
        tex.draw(rd.set(rd.get().r2, 1));
    } else if (mDesc) {
        tex.draw(rd.set(descData.getDest()));

        tex.draw(descData);

        rd.mColor = BLACK;
        tex.draw(rd.set(rd.get().r2, 1));
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
    mEffectSub = ParameterSystem::subscribe(
        valueParams, stateParams, [this, func]() { setInfo(func()); });
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
                    ParameterSystem::ValueParam cost)
    : mMoney(money), mCost(cost) {}
Upgrade::Cost::Cost(ParameterSystem::BaseValue level,
                    ParameterSystem::BaseValue money,
                    ParameterSystem::NodeValue cost,
                    std::function<Number(const Number&)> costFunc)
    : Cost(money, cost) {
    mCostSub = cost.subscribeTo(level, costFunc);
}

const ParameterSystem::ValueParam& Upgrade::Cost::getCostParam() const {
    return mCost;
}
const ParameterSystem::BaseValue& Upgrade::Cost::getMoneyParam() const {
    return mMoney;
}
const Number& Upgrade::Cost::getCost() const { return mCost.get(); }
const Number& Upgrade::Cost::getMoney() const { return mMoney.get(); }
bool Upgrade::Cost::canBuy() const { return mCost.get() <= mMoney.get(); }
void Upgrade::Cost::buy() const { mMoney.set(mMoney.get() - mCost.get()); }

ParameterSystem::ParameterSubscriptionPtr Upgrade::Cost::subscribe(
    std::function<void()> func) const {
    ParameterSystem::ParameterSubscriptionPtr sub = mCost.subscribe(func);
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
    mMaxLevelSub = maxLevel.subscribe([this](const Number& lvl) {
        mMaxLevel = std::max(0, lvl.toInt());
        updateInfo();
    });
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
                      ParameterSystem::ValueParam cost) {
    mCost = std::make_unique<Cost>(money, cost);
    mCostSub = mCost->subscribe([this]() { updateInfo(); });
}
void Upgrade::setCost(ParameterSystem::BaseValue money,
                      ParameterSystem::NodeValue cost,
                      std::function<Number(const Number&)> costFunc) {
    mCost = std::make_unique<Cost>(mLevel, money, cost, costFunc);
    mCostSub = mCost->subscribe([this]() { updateInfo(); });
}
void Upgrade::clearCost() {
    mCost.reset();
    mCostSub.reset();
    updateInfo();
}

void Upgrade::setEffect(ParameterSystem::NodeValue param, ValueFunc func,
                        std::function<std::string(const Number&)> effectFunc) {
    if (effectFunc) {
        setEffects({{param, func}}, {},
                   [param, effectFunc]() { return effectFunc(param.get()); });
    } else {
        setEffects({{param, func}}, {});
    }
}
void Upgrade::setEffect(ParameterSystem::NodeState param, StateFunc func,
                        std::function<std::string(bool)> effectFunc) {
    if (effectFunc) {
        setEffects({}, {{param, func}},
                   [param, effectFunc]() { return effectFunc(param.get()); });
    } else {
        setEffects({}, {{param, func}});
    }
}
void Upgrade::setEffects(
    std::initializer_list<std::pair<ParameterSystem::NodeValue, ValueFunc>>
        values,
    std::initializer_list<std::pair<ParameterSystem::NodeState, StateFunc>>
        states,
    std::function<std::string()> func) {
    mEffectLevelSubs.clear();
    mEffectSub.reset();
    for (auto param : values) {
        mEffectLevelSubs.push_back(
            param.first.subscribeTo(mLevel, param.second));
        if (func) {
            if (!mEffectSub) {
                mEffectSub = param.first.subscribe([this, func]() {
                    mEffectStr = func();
                    updateInfo();
                });
            } else {
                param.first->subscribe(mEffectSub);
            }
        }
    }
    for (auto param : states) {
        mEffectLevelSubs.push_back(
            param.first.subscribeTo(mLevel, param.second));
        if (func) {
            if (!mEffectSub) {
                mEffectSub = param.first.subscribe([this, func]() {
                    mEffectStr = func();
                    updateInfo();
                });
            } else {
                param.first->subscribe(mEffectSub);
            }
        }
    }
}
void Upgrade::clearEffects() {
    mEffectLevelSubs.clear();
    mEffectSub.reset();
    mEffectStr = "";
    updateInfo();
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
                ss << "{i" << Money::GetMoneyIcon(mCost->getMoneyParam()) << "}"
                   << mCost->getCost();
            }
        } else {
            ss << (mMaxLevel > 1 ? "Maxed" : "Bought");
        }
    }
    setInfo(ss.str());
}
