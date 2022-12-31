#include "Upgrade.h"

// UpgradeSnapshot
bool UpgradeSnapshot::operator!=(const UpgradeSnapshot& rhs) const {
    return mMaxLvl != rhs.mMaxLvl;
}

// UpgradeCost
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
RenderTextureCPtr UpgradeCost::getMoneyIcon() const {
    return MoneyIcons::Get(mMoney);
}
bool UpgradeCost::canBuy() const { return mCost.get() <= mMoney.get(); }
void UpgradeCost::buy() const { mMoney.set(mMoney.get() - mCost.get()); }

ParameterSystem::ParameterSubscriptionPtr UpgradeCost::subscribe(
    std::function<void()> func) const {
    return mCost.subscribe(func);
}

// UpgradeDefaults
namespace UpgradeDefaults {
const ParameterSystem::BaseValue
    CRYSTAL_MAGIC = Crystal::Params::get(Crystal::Param::Magic),
    CRYSTAL_SHARDS = Crystal::Params::get(Crystal::Param::Shards),
    CATALYST_MAGIC = Catalyst::Params::get(Catalyst::Param::Magic),
    ROBOT_SHARDS = RobotWizard::Params::get(RobotWizard::Param::Shards);

std::string NoEffectText(const Number& effect) { return effect.toString(); }
TextUpdateData NoEffect(const Number& effect) { return {NoEffectText(effect)}; }
std::string AdditiveEffectText(const Number& effect) {
    return "+" + effect.toString();
}
TextUpdateData AdditiveEffect(const Number& effect) {
    return {AdditiveEffectText(effect)};
}
std::string MultiplicativeEffectText(const Number& effect) {
    return effect.toString() + "x";
}
TextUpdateData MultiplicativeEffect(const Number& effect) {
    return {MultiplicativeEffectText(effect)};
}
std::string PercentEffectText(const Number& effect) {
    return (effect * 100).toString() + "%";
}
TextUpdateData PercentEffect(const Number& effect) {
    return {PercentEffectText(effect)};
}
std::string PowerEffectText(const Number& effect) {
    return "^" + effect.toString();
}
TextUpdateData PowerEffect(const Number& effect) {
    return {PowerEffectText(effect)};
}

ParameterSystem::ParameterSubscriptionPtr subscribeT1UpCost(
    ParameterSystem::BaseValue lvlParam, ParameterSystem::NodeValue costParam,
    std::function<Number(const Number&)> costFunc) {
    auto costMult = Crystal::Params::get(Crystal::Param::T1CostMult);
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
        text.texture.setFit(RenderData::FitMode::Texture);
        text.texture.setFitAlign(Rect::CENTER, Rect::TOP_LEFT);
    }
}

UpgradeBase::Status UpgradeBase::status(bool free) {
    Status uStatus = _status();

    if (uStatus == Status::CAN_BUY && !free && mCost && !mCost->canBuy()) {
        return Status::CANT_BUY;
    }

    return uStatus;
}

UpgradeBase::Status UpgradeBase::_status() { return Status::NOT_BUYABLE; }

void UpgradeBase::buy(bool free) {
    if (!free) {
        if (status() != Status::CAN_BUY) {
            return;
        }

        if (mCost) {
            mCost->buy();
        }
    }

    _buy();
}

void UpgradeBase::_buy() {}

void UpgradeBase::max(bool free) {
    if (free) {
        _max();
    } else {
        while (status() == Status::CAN_BUY) {
            buy();
        }
    }
}

void UpgradeBase::_max() {}

UpgradeSnapshot UpgradeBase::getSnapshot() const { return {0}; }

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
void UpgradeBase::setEffectText(const std::string& text) {
    updateDescText(DescType::Effect, text);
}
void UpgradeBase::setEffectImgs(const std::vector<RenderTextureCPtr>& imgs) {
    updateDescImgs(DescType::Effect, imgs);
}

void UpgradeBase::drawIcon(TextureBuilder& tex, const Rect& r,
                           UpgradeDrawOptions opts, const Rect& bounds) {
    RectShape rd;
    rd.boundary = bounds;
    if (opts.bkgrnd) {
        rd.mColor = GRAY;
        tex.draw(rd.set(r));
    }
    if (opts.border) {
        switch (status()) {
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
            default:
                rd.mColor = GRAY;
                break;
        }
        tex.draw(rd.set(r, (int)(r.minDim() / 15 + .5)));
    }

    mImg.setDest(r);
    mImg.setBoundary(bounds);
    tex.draw(mImg);
}

void UpgradeBase::drawDescription(TextureBuilder tex, SDL_FPoint offset) {
    float w = 0, h = 0;
    for (auto& desc : mDescText) {
        desc.texture.set(desc.text);
        desc.texture.setDest(Rect(offset.x, offset.y + h, 0, 0));
        SDL_Point dim = desc.texture.get()->getTextureDim();
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
    updateDescText(type, text.text);
    updateDescImgs(type, text.imgs);
}
void UpgradeBase::updateDescText(DescType type, const std::string& text) {
    mDescText[type].text->setText(text, GetDescWidth());
}
void UpgradeBase::updateDescImgs(DescType type,
                                 const std::vector<RenderTextureCPtr>& imgs) {
    mDescText[type].text->setImgs(imgs);
}

// Cost
const std::unique_ptr<UpgradeCost>& UpgradeBase::getCost() const {
    return mCost;
}
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
UpgradeBase::Status Display::_status() { return NOT_BUYABLE; }

// Toggle
Toggle::Toggle(LevelFunc onLevel, unsigned int numStates)
    : mOnLevel(onLevel), mNumStates(numStates) {
    setLevel(0);
}

UpgradeBase::Status Toggle::_status() { return CAN_BUY; }

void Toggle::_buy() {
    mLevel = mNumStates == 0 ? 0 : (mLevel + 1) % mNumStates;
    mOnLevel(mLevel, *this);
}

void Toggle::setLevel(unsigned int lvl) {
    mLevel = mNumStates == 0 ? 0 : lvl % mNumStates;
    mOnLevel(mLevel, *this);
}

// Unlockable
Unlockable::Unlockable(ParameterSystem::BaseState level) : mLevel(level) {
    mLevelSub = mLevel.subscribe(
        [this](bool val) { updateDesc(DescType::Cost, getCostText()); });
}

UpgradeSnapshot Unlockable::getSnapshot() const { return {1}; }

UpgradeBase::Status Unlockable::_status() {
    if (mLevel.get()) {
        return BOUGHT;
    }
    return CAN_BUY;
}

void Unlockable::_buy() { mLevel.set(!mLevel.get()); }

void Unlockable::_max() { mLevel.set(true); }

TextUpdateData Unlockable::getCostText() const {
    std::vector<RenderTextureCPtr> imgs;
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
    : mLevel(level), mMaxLevel(maxLevel) {
    mLevelSub = mLevel.subscribe([this](const Number& val) {
        updateDesc(DescType::Cost, getCostText());
    });
}
Upgrade::Upgrade(ParameterSystem::BaseValue level,
                 ParameterSystem::ValueParam maxLevel)
    : Upgrade(level, std::max(0, maxLevel.get().toInt())) {
    mMaxLevelSub = maxLevel.subscribe([this](const Number& lvl) {
        mMaxLevel = std::max(0, lvl.toInt());
        updateDesc(DescType::Cost, getCostText());
    });
}

UpgradeSnapshot Upgrade::getSnapshot() const { return {mMaxLevel}; }

Upgrade::Status Upgrade::_status() {
    if (mMaxLevel == 0) {
        return NOT_BUYABLE;
    }
    if (mLevel.get() >= mMaxLevel) {
        return BOUGHT;
    }
    return CAN_BUY;
}

void Upgrade::_buy() { mLevel.set(mLevel.get() + 1); }

void Upgrade::_max() { mLevel.set(mMaxLevel); }

TextUpdateData Upgrade::getCostText() const {
    std::vector<RenderTextureCPtr> imgs;
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
