#ifndef UPGRADE_H
#define UPGRADE_H

#include <RenderSystem/RenderSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem.h>
#include <Wizards/Money.h>

#include <array>
#include <initializer_list>
#include <list>
#include <memory>

struct TextUpdateData {
    std::string text = "";
    std::vector<RenderDataWPtr> imgs;
};

struct UpgradeCost {
   public:
    UpgradeCost(ParameterSystem::BaseValue money,
                ParameterSystem::ValueParam cost);

    const ParameterSystem::ValueParam& getCostParam() const;
    const ParameterSystem::BaseValue& getMoneyParam() const;
    const Number& getCost() const;
    const Number& getMoney() const;
    RenderDataWPtr getMoneyIcon() const;
    bool canBuy() const;
    void buy() const;

    ParameterSystem::ParameterSubscriptionPtr subscribe(
        std::function<void()> func) const;

   private:
    ParameterSystem::ValueParam mCost;
    ParameterSystem::BaseValue mMoney;
};

namespace UpgradeDefaults {
extern const ParameterSystem::BaseValue CRYSTAL_MAGIC, CRYSTAL_SHARDS;

TextUpdateData NoEffect(const Number& effect);
TextUpdateData AdditiveEffect(const Number& effect);
TextUpdateData MultiplicativeEffect(const Number& effect);
TextUpdateData PercentEffect(const Number& effect);
TextUpdateData PowerEffect(const Number& effect);

ParameterSystem::ParameterSubscriptionPtr subscribeT1UpCost(
    ParameterSystem::BaseValue lvlParam, ParameterSystem::NodeValue costParam,
    std::function<Number(const Number&)> costFunc);
}  // namespace UpgradeDefaults

class UpgradeBase {
   public:
    enum Status : uint8_t {
        BOUGHT = 0,
        CAN_BUY,
        CANT_BUY,
        NOT_BUYABLE,
    };

    UpgradeBase();
    virtual ~UpgradeBase() = default;

    virtual Status getStatus();
    virtual void buy();
    virtual void maxFree();

    void setImage(WizardId id);
    void setImage(const std::string& file);
    void setDescription(const TextUpdateData& data);
    void setEffect(const TextUpdateData& data);

    void drawIcon(TextureBuilder& tex, const Rect& r);
    void drawDescription(TextureBuilder tex, SDL_FPoint offset = {0, 0});

    const static SDL_Color DESC_BKGRND;
    const static FontData DESC_FONT;

    // Cost
    const std::unique_ptr<UpgradeCost>& getCost() const;
    void setCost(ParameterSystem::BaseValue money,
                 ParameterSystem::ValueParam cost);
    virtual TextUpdateData getCostText() const;

    // Effects
    void setEffects(ParameterSystem::ValueParam param,
                    std::function<TextUpdateData(const Number&)> func);
    void setEffects(ParameterSystem::StateParam param,
                    std::function<TextUpdateData(bool)> func);
    void setEffects(
        const std::initializer_list<ParameterSystem::ValueParam>& values,
        const std::initializer_list<ParameterSystem::StateParam>& states,
        std::function<TextUpdateData()> func);

   protected:
    enum DescType : uint8_t {
        Desc = 0,
        Effect,
        Cost,

        size
    };

    void updateDesc(DescType type, const TextUpdateData& text);

    std::unique_ptr<UpgradeCost> mCost;
    ParameterSystem::ParameterSubscriptionPtr mCostSub, mEffectSub;

   private:
    static int GetDescWidth();

    RenderData mImg;
    struct DescText {
        TextDataPtr text;
        RenderData texture;
    };
    std::array<DescText, DescType::size> mDescText;

    WizardSystem::WizardImageObservable::IdSubscriptionPtr mWizImgSub;
};

typedef std::shared_ptr<UpgradeBase> UpgradeBasePtr;

// Upgrade that receives no interaction
class Display : public UpgradeBase {
   public:
    virtual ~Display() = default;

    virtual Status getStatus();

   protected:
    using UpgradeBase::setCost;
};

typedef std::shared_ptr<Display> DisplayPtr;

// Upgrade that loops through a set number of states
class Toggle : public Display {
    typedef std::function<void(unsigned int, Toggle&)> LevelFunc;

   public:
    Toggle(LevelFunc onLevel, unsigned int numStates = 1);

    Status getStatus();

    void buy();

    void setLevel(unsigned int lvl);

   private:
    unsigned int mLevel = 0;
    unsigned int mNumStates;
    LevelFunc mOnLevel;
};

typedef std::shared_ptr<Toggle> TogglePtr;

// Upgrade that can only be bought once
class Unlockable : public Display {
   public:
    Unlockable(ParameterSystem::BaseState level);

    Status getStatus();
    void buy();
    void maxFree();

    TextUpdateData getCostText() const;

    ParameterSystem::BaseState level() const;

    using UpgradeBase::setCost;

   private:
    // Level
    ParameterSystem::BaseState mLevel;
    ParameterSystem::ParameterSubscriptionPtr mLevelSub;
};

typedef std::shared_ptr<Unlockable> UnlockablePtr;

// Flexible upgrade
class Upgrade : public Display {
   public:
    typedef std::function<Number(const Number&)> ValueFunc;
    typedef std::function<bool(const Number&)> StateFunc;

    Upgrade(ParameterSystem::BaseValue level, unsigned int maxLevel);
    Upgrade(ParameterSystem::BaseValue level,
            ParameterSystem::ValueParam maxLevel);

    Status getStatus();
    void buy();
    void maxFree();

    TextUpdateData getCostText() const;

    ParameterSystem::BaseValue level() const;

    using UpgradeBase::setCost;

   private:
    // Level
    ParameterSystem::BaseValue mLevel;
    unsigned int mMaxLevel;
    ParameterSystem::ParameterSubscriptionPtr mLevelSub, mMaxLevelSub;
};

typedef std::shared_ptr<Upgrade> UpgradePtr;

#endif
