#include "Empty.h"

// UpgradeBase
const SDL_Color UpgradeBase::DESC_BKGRND{175, 175, 175, 255};
const FontData UpgradeBase::DESC_FONT{-1, 20, "|"};

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

// Cost
Upgrade::Cost::Cost(ParameterSystem::BaseValue level,
                    ParameterSystem::NodeValue cost,
                    ParameterSystem::BaseValue money,
                    std::function<Number(const Number&)> costFunc)
    : mCost(cost), mMoney(money), mCostSub(cost.subscribeTo(level, costFunc)) {}

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
    mInfoStr = func();
    mUpdateInfo = true;
    mEffectSub =
        ParameterSystem::subscribe(valueParams, stateParams, [this, func]() {
            mInfoStr = func();
            mUpdateInfo = true;
        });
}

// Toggle
Toggle::Toggle(LevelFunc onLevel, unsigned int numStates)
    : mOnLevel(onLevel), mNumStates(numStates) {}

UpgradeBase::Status Toggle::getStatus() { return CAN_BUY; }

void Toggle::buy() {
    mLevel = mNumStates == 0 ? 0 : (mLevel + 1) % mNumStates;
    mOnLevel(mLevel, *this);
}

void Toggle::setLevel(unsigned int lvl) {
    mLevel = mNumStates == 0 ? 0 : lvl % mNumStates;
    mOnLevel(mLevel, *this);
}

// Upgrade
Upgrade::Upgrade(ParameterSystem::BaseValue level, int maxLevel)
    : mLevel(level), mMaxLevel(maxLevel) {}
Upgrade::Upgrade(ParameterSystem::BaseValue level,
                 ParameterSystem::ValueParam maxLevel)
    : mLevel(level), mMaxLevelSub(maxLevel.subscribe([this](const Number& lvl) {
          mMaxLevel = (int)lvl.toFloat();
      })) {}

void Upgrade::setCost(ParameterSystem::NodeValue cost,
                      ParameterSystem::BaseValue money,
                      std::function<Number(const Number&)> costFunc) {
    mCost = std::make_unique<Cost>(mLevel, cost, money, costFunc);
}
void Upgrade::clearCost() { mCost.reset(); }
