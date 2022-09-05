#include "Upgrade.h"

// Cost
UpgradeCost::UpgradeCost(ParameterSystem::BaseValue money,
                         ParameterSystem::ValueParam cost)
    : mMoney(money), mCost(cost) {}

const ParameterSystem::ValueParam& UpgradeCost::getCostParam() const {
    return mCost;
}
const ParameterSystem::BaseValue& UpgradeCost::getMoneyParam() const {
    return mMoney;
}
const Number& UpgradeCost::getCost() const { return mCost.get(); }
const Number& UpgradeCost::getMoney() const { return mMoney.get(); }
RenderDataWPtr UpgradeCost::getMoneyIcon() const {
    return Money::GetMoneyIcon(mMoney);
}
bool UpgradeCost::canBuy() const { return mCost.get() <= mMoney.get(); }
void UpgradeCost::buy() const { mMoney.set(mMoney.get() - mCost.get()); }

ParameterSystem::ParameterSubscriptionPtr UpgradeCost::subscribe(
    std::function<void()> func) const {
    return mCost.subscribe(func);
}

// UpgradeDefaults
namespace UpgradeDefaults {
const ParameterSystem::BaseValue CRYSTAL_MAGIC =
    ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic);
const ParameterSystem::BaseValue CRYSTAL_SHARDS =
    ParameterSystem::Param<CRYSTAL>(CrystalParams::Shards);

TextUpdateData AdditiveEffect(const Number& effect) {
    return {"+" + effect.toString()};
}
TextUpdateData MultiplicativeEffect(const Number& effect) {
    return {effect.toString() + "x"};
}
TextUpdateData PercentEffect(const Number& effect) {
    return {(effect * 100).toString() + "%"};
}
TextUpdateData PowerEffect(const Number& effect) {
    return {"^" + effect.toString()};
}

ParameterSystem::ParameterSubscriptionPtr subscribeT1UpCost(
    ParameterSystem::BaseValue lvlParam, ParameterSystem::NodeValue costParam,
    std::function<Number(const Number&)> costFunc) {
    auto costMult = ParameterSystem::Param<CRYSTAL>(CrystalParams::T1CostMult);
    return costParam.subscribeTo(
        {lvlParam, costMult}, {}, [costFunc, lvlParam, costMult]() {
            return costFunc(lvlParam.get()) * costMult.get();
        });
}
}  // namespace UpgradeDefaults

// UpgradeBase
const SDL_Color UpgradeBase::DESC_BKGRND{175, 175, 175, 255};
const FontData UpgradeBase::DESC_FONT{-1, 22, "|"};

int UpgradeBase::GetDescWidth() { return RenderSystem::getWindowSize().x / 3; }

UpgradeBase::UpgradeBase() {
    for (auto& text : mDescText) {
        text = {std::make_shared<TextData>()};
        text.text->setFont(DESC_FONT);
        text.texture.setFit(RenderData::FitMode::Texture)
            .setFitAlign(Rect::CENTER, Rect::TOP_LEFT);
    }
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
    updateDesc(DescType::Desc, data);
}

void UpgradeBase::setEffect(const TextUpdateData& data) {
    updateDesc(DescType::Effect, data);
}

void UpgradeBase::drawIcon(TextureBuilder& tex, const Rect& r) {
    tex.draw(mImg.setDest(r));
}

void UpgradeBase::drawDescription(TextureBuilder tex, SDL_FPoint offset) {
    float w = 0, h = 0;
    for (auto& desc : mDescText) {
        desc.texture.set(desc.text).setDest(Rect(offset.x, offset.y + h, 0, 0));
        SDL_Point dim = desc.texture.getTextureDim();
        if (w < dim.x) {
            w = dim.x;
        }
        h += dim.y;
    }

    if (w == 0 || h == 0) {
        return;
    }

    Rect r(offset.x - w / 2, offset.y, w, h);
    RectShape rd = RectShape(DESC_BKGRND).set(r);
    tex.draw(rd);

    for (auto& desc : mDescText) {
        tex.draw(desc.texture);
    }

    rd.mColor = BLACK;
    rd.set(r, 1);
    tex.draw(rd);
}

void UpgradeBase::updateDesc(DescType type, const TextUpdateData& text) {
    mDescText[type].text->setText(text.text, GetDescWidth()).setImgs(text.imgs);
}

// Cost
void UpgradeBase::setCost(ParameterSystem::BaseValue money,
                          ParameterSystem::ValueParam cost) {
    mCost = std::make_unique<UpgradeCost>(money, cost);
    mCostSub = mCost->subscribe(
        [this]() { updateDesc(DescType::Cost, getCostText()); });
}

TextUpdateData UpgradeBase::getCostText() const { return {}; }

// Effect
void UpgradeBase::setEffects(
    ParameterSystem::ValueParam param,
    std::function<TextUpdateData(const Number&)> func) {
    mEffectSub = param.subscribe([this, param, func](const Number& val) {
        updateDesc(DescType::Effect, func(val));
    });
}
void UpgradeBase::setEffects(ParameterSystem::StateParam param,
                             std::function<TextUpdateData(bool)> func) {
    mEffectSub = param.subscribe([this, param, func](bool val) {
        updateDesc(DescType::Effect, func(val));
    });
}
void UpgradeBase::setEffects(
    const std::initializer_list<ParameterSystem::ValueParam>& values,
    const std::initializer_list<ParameterSystem::StateParam>& states,
    std::function<TextUpdateData()> func) {
    mEffectSub = ParameterSystem::subscribe(values, states, [this, func]() {
        updateDesc(DescType::Effect, func());
    });
}

// Display
UpgradeBase::Status Display::getStatus() { return NOT_BUYABLE; }

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

// Unlockable
Unlockable::Unlockable(ParameterSystem::BaseState level) : mLevel(level) {}

UpgradeBase::Status Unlockable::getStatus() {
    if (mLevel.get()) {
        return BOUGHT;
    }
    return mCost && !mCost->canBuy() ? CANT_BUY : CAN_BUY;
}

void Unlockable::buy() {
    if (getStatus() != CAN_BUY) {
        return;
    }

    if (mCost) {
        mCost->buy();
    }
    mLevel.set(!mLevel.get());
}

TextUpdateData Unlockable::getCostText() const {
    std::vector<RenderDataWPtr> imgs;
    std::stringstream ss;
    if (mLevel.get()) {
        ss << "Bought";
    } else if (mCost) {
        ss << "{i}" << mCost->getCost();
        imgs.push_back(mCost->getMoneyIcon());
    }
    return {ss.str(), imgs};
}

ParameterSystem::BaseState Unlockable::level() const { return mLevel; }

// Upgrade
Upgrade::Upgrade(ParameterSystem::BaseValue level, unsigned int maxLevel)
    : mLevel(level), mMaxLevel(maxLevel) {}
Upgrade::Upgrade(ParameterSystem::BaseValue level,
                 ParameterSystem::ValueParam maxLevel)
    : Upgrade(level, std::max(0, maxLevel.get().toInt())) {
    mMaxLevelSub = maxLevel.subscribe([this](const Number& lvl) {
        mMaxLevel = std::max(0, lvl.toInt());
        updateDesc(DescType::Cost, getCostText());
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

TextUpdateData Upgrade::getCostText() const {
    std::vector<RenderDataWPtr> imgs;
    std::stringstream ss;
    if (mMaxLevel > 0) {
        Number lvl = mLevel.get();
        if (mMaxLevel > 1) {
            ss << lvl << "/" << mMaxLevel << ": ";
        }
        if (lvl < mMaxLevel) {
            if (mCost) {
                imgs.push_back(mCost->getMoneyIcon());
                ss << "{i}" << mCost->getCost();
            }
        } else {
            ss << "Maxed";
        }
    }
    return {ss.str(), imgs};
}

ParameterSystem::BaseValue Upgrade::level() const { return mLevel; }
