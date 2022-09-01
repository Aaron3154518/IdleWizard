#include "Upgrade.h"

// Cost
UpgradeBase::Cost::Cost(ParameterSystem::BaseValue money,
                        ParameterSystem::ValueParam cost)
    : mMoney(money), mCost(cost) {}
UpgradeBase::Cost::Cost(ParameterSystem::BaseValue level,
                        ParameterSystem::BaseValue money,
                        ParameterSystem::NodeValue cost,
                        std::function<Number(const Number&)> costFunc)
    : Cost(money, cost) {
    mCostSub = cost.subscribeTo(level, costFunc);
}

const ParameterSystem::ValueParam& UpgradeBase::Cost::getCostParam() const {
    return mCost;
}
const ParameterSystem::BaseValue& UpgradeBase::Cost::getMoneyParam() const {
    return mMoney;
}
const Number& UpgradeBase::Cost::getCost() const { return mCost.get(); }
const Number& UpgradeBase::Cost::getMoney() const { return mMoney.get(); }
const RenderDataPtr& UpgradeBase::Cost::getMoneyIcon() const {
    return Money::GetMoneyIcon(mMoney);
}
bool UpgradeBase::Cost::canBuy() const { return mCost.get() <= mMoney.get(); }
void UpgradeBase::Cost::buy() const { mMoney.set(mMoney.get() - mCost.get()); }

ParameterSystem::ParameterSubscriptionPtr UpgradeBase::Cost::subscribe(
    std::function<void()> func) const {
    ParameterSystem::ParameterSubscriptionPtr sub = mCost.subscribe(func);
    return sub;
}

// UpgradeBase
const SDL_Color UpgradeBase::DESC_BKGRND{175, 175, 175, 255};
const FontData UpgradeBase::DESC_FONT{-1, 22, "|"};

const ParameterSystem::BaseValue UpgradeBase::Defaults::CRYSTAL_MAGIC =
    ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic);
const ParameterSystem::BaseValue UpgradeBase::Defaults::CRYSTAL_SHARDS =
    ParameterSystem::Param<CRYSTAL>(CrystalParams::Shards);

TextUpdateData Upgrade::Defaults::AdditiveEffect(const Number& effect) {
    return {"+" + effect.toString()};
}
TextUpdateData Upgrade::Defaults::MultiplicativeEffect(const Number& effect) {
    return {effect.toString() + "x"};
}
TextUpdateData Upgrade::Defaults::PercentEffect(const Number& effect) {
    return {(effect * 100).toString() + "%"};
}

int UpgradeBase::GetDescWidth() { return RenderSystem::getWindowSize().x / 3; }

UpgradeBase::UpgradeBase() {
    mDescText->setFont(DESC_FONT);
    mDesc.set(mDescText)
        .setFit(RenderData::FitMode::Texture)
        .setFitAlign(Rect::CENTER, Rect::TOP_LEFT);

    mInfoText->setFont(DESC_FONT);
    mInfo.set(mInfoText)
        .setFit(RenderData::FitMode::Texture)
        .setFitAlign(Rect::CENTER, Rect::TOP_LEFT);
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

void UpgradeBase::setDescription(const TextUpdateData& data) {
    mDescText->setText(data.text, GetDescWidth()).setImgs(data.imgs);
}

void UpgradeBase::setInfo(const TextUpdateData& data) {
    mInfoText->setText(data.text, GetDescWidth()).setImgs(data.imgs);
}

void UpgradeBase::drawIcon(TextureBuilder& tex, const Rect& r) {
    tex.draw(mImg.setDest(r));
}

void UpgradeBase::drawDescription(TextureBuilder tex, SDL_FPoint offset) {
    RectShape rd = RectShape(DESC_BKGRND);

    mDesc.setDest(Rect(offset.x, offset.y, 0, 0));
    mInfo.setDest(Rect(0, 0, 0, 0));
    float w = mInfo.getDest().w();
    w = std::max(w, mDesc.getDest().w());
    mInfo.setDest(Rect(mDesc.getDest().cX(), mDesc.getDest().y2(), 0, 0));

    Rect rdRect(0, offset.y, w, mInfo.getDest().y2() - offset.y);
    rdRect.setPosX(offset.x, Rect::CENTER);
    if (!rdRect.empty()) {
        tex.draw(rd.set(rdRect));
    }

    tex.draw(mInfo);
    tex.draw(mDesc);

    if (!rdRect.empty()) {
        rd.mColor = BLACK;
        tex.draw(rd.set(rd.get().r2, 1));
    }
}

// Display
UpgradeBase::Status Display::getStatus() { return NOT_BUYABLE; }

void Display::setEffect(ParameterSystem::ValueParam param,
                        std::function<TextUpdateData(const Number&)> func) {
    setEffects({param}, {}, [param, func]() { return func(param.get()); });
}
void Display::setEffect(ParameterSystem::StateParam param,
                        std::function<TextUpdateData(bool)> func) {
    setEffects({}, {param}, [param, func]() { return func(param.get()); });
}
void Display::setEffects(
    const std::initializer_list<ParameterSystem::ValueParam>& valueParams,
    const std::initializer_list<ParameterSystem::StateParam>& stateParams,
    std::function<TextUpdateData()> func) {
    mEffectSub =
        ParameterSystem::subscribe(valueParams, stateParams, [this, func]() {
            mEffectText = func();
            updateInfo();
        });
}

void Display::updateInfo() { setInfo(mEffectText); }

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

// Buyable
Buyable::Buyable(ParameterSystem::BaseState level,
                 std::function<void(bool)> onLevel)
    : mLevel(level) {
    mLevelSub = level.subscribe([this, onLevel]() {
        onLevel(mLevel.get());
        updateInfo();
    });
}

UpgradeBase::Status Buyable::getStatus() {
    if (mLevel.get()) {
        return BOUGHT;
    }
    return mCost && !mCost->canBuy() ? CANT_BUY : CAN_BUY;
}

void Buyable::buy() {
    if (getStatus() != CAN_BUY) {
        return;
    }

    if (mCost) {
        mCost->buy();
    }
    mLevel.set(!mLevel.get());
}

void Buyable::setCost(ParameterSystem::BaseValue money,
                      ParameterSystem::ValueParam cost) {
    mCost = std::make_unique<Cost>(money, cost);
    mCostSub = mCost->subscribe([this]() { updateInfo(); });
}
void Buyable::clearCost() {
    mCost.reset();
    mCostSub.reset();
    updateInfo();
}

void Buyable::updateInfo() {
    std::vector<RenderDataWPtr> imgs = mEffectText.imgs;
    std::stringstream ss;
    ss << mEffectText.text;
    if (!mEffectText.text.empty()) {
        ss << "\n";
    }
    if (mLevel.get()) {
        ss << "Bought";
    } else if (mCost) {
        ss << "{i}" << mCost->getCost();
        imgs.push_back(mCost->getMoneyIcon());
    }
    setInfo({ss.str(), imgs});
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

void Upgrade::setEffect(
    ParameterSystem::NodeValue param, ValueFunc func,
    std::function<TextUpdateData(const Number&)> effectFunc) {
    if (effectFunc) {
        setEffects({{param, func}}, {},
                   [param, effectFunc]() { return effectFunc(param.get()); });
    } else {
        setEffects({{param, func}}, {});
    }
}
void Upgrade::setEffect(ParameterSystem::NodeState param, StateFunc func,
                        std::function<TextUpdateData(bool)> effectFunc) {
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
    std::function<TextUpdateData()> func) {
    mEffectLevelSubs.clear();
    mEffectSub.reset();
    for (auto param : values) {
        mEffectLevelSubs.push_back(
            param.first.subscribeTo(mLevel, param.second));
        if (func) {
            if (!mEffectSub) {
                mEffectSub = param.first.subscribe([this, func]() {
                    mEffectText = func();
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
                    mEffectText = func();
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
    mEffectText = {};
    updateInfo();
}

void Upgrade::updateInfo() {
    std::stringstream ss;
    ss << mEffectText.text;
    if (mMaxLevel > 0) {
        Number lvl = mLevel.get();
        if (!mEffectText.text.empty()) {
            ss << "\n";
        }
        if (mMaxLevel > 1) {
            ss << lvl << "/" << mMaxLevel << ": ";
        }
        if (lvl < mMaxLevel) {
            if (mCost) {
                ss << "{i}" << mCost->getCost();
            }
        } else {
            ss << "Maxed";
        }
    }
    std::vector<RenderDataWPtr> imgs = mEffectText.imgs;
    if (mCost) {
        imgs.push_back(mCost->getMoneyIcon());
    }
    setInfo({ss.str(), imgs});
}
