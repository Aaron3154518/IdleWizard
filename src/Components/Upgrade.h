#ifndef UPGRADE_H
#define UPGRADE_H

#include <RenderSystem/RenderSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem.h>
#include <Wizards/Money.h>

#include <initializer_list>
#include <list>
#include <memory>

struct TextUpdateData {
    std::string text = "";
    std::vector<RenderDataWPtr> imgs;
};

class UpgradeBase {
   public:
    struct Defaults {
        const static ParameterSystem::BaseValue CRYSTAL_MAGIC, CRYSTAL_SHARDS;

        static TextUpdateData AdditiveEffect(const Number& effect);
        static TextUpdateData MultiplicativeEffect(const Number& effect);
        static TextUpdateData PercentEffect(const Number& effect);
    };

    struct Cost {
       public:
        Cost(ParameterSystem::BaseValue money,
             ParameterSystem::ValueParam cost);
        Cost(ParameterSystem::BaseValue level, ParameterSystem::BaseValue money,
             ParameterSystem::NodeValue cost,
             std::function<Number(const Number&)> costFunc);

        const ParameterSystem::ValueParam& getCostParam() const;
        const ParameterSystem::BaseValue& getMoneyParam() const;
        const Number& getCost() const;
        const Number& getMoney() const;
        const RenderDataPtr& getMoneyIcon() const;
        bool canBuy() const;
        void buy() const;

        ParameterSystem::ParameterSubscriptionPtr subscribe(
            std::function<void()> func) const;

       private:
        ParameterSystem::ValueParam mCost;
        ParameterSystem::BaseValue mMoney;
        ParameterSystem::ParameterSubscriptionPtr mCostSub;
    };

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

    void setImage(WizardId id);
    void setImage(const std::string& file);
    void setDescription(const TextUpdateData& data);
    void setInfo(const TextUpdateData& data);

    void drawIcon(TextureBuilder& tex, const Rect& r);
    void drawDescription(TextureBuilder tex, SDL_FPoint offset = {0, 0});

    const static SDL_Color DESC_BKGRND;
    const static FontData DESC_FONT;

   protected:
    static int GetDescWidth();

    TextDataPtr mDescText = std::make_shared<TextData>(),
                mInfoText = std::make_shared<TextData>();

   private:
    RenderData mImg, mDesc, mInfo;
    WizardSystem::WizardImageObservable::IdSubscriptionPtr mWizImgSub;
};

typedef std::shared_ptr<UpgradeBase> UpgradeBasePtr;

// Upgrade that receives no interaction
class Display : public UpgradeBase {
   public:
    virtual ~Display() = default;

    virtual Status getStatus();

    void setEffect(ParameterSystem::ValueParam param,
                   std::function<TextUpdateData(const Number&)> func);
    void setEffect(ParameterSystem::StateParam param,
                   std::function<TextUpdateData(bool)> func);
    void setEffects(
        const std::initializer_list<ParameterSystem::ValueParam>& valueParams,
        const std::initializer_list<ParameterSystem::StateParam>& stateParams,
        std::function<TextUpdateData()> func);

    virtual void updateInfo();

   protected:
    // Effects
    TextUpdateData mEffectText;
    ParameterSystem::ParameterSubscriptionPtr mEffectSub;
};

typedef std::shared_ptr<Display> DisplayPtr;

// Upgrade that loops through a set number of states
class Toggle : public Display {
   public:
    typedef std::function<void(unsigned int, Toggle&)> LevelFunc;

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
class Buyable : public Display {
   public:
    Buyable(
        ParameterSystem::BaseState level,
        std::function<void(bool)> onLevel = [](bool b) {});

    Status getStatus();
    void buy();

    void setCost(ParameterSystem::BaseValue money,
                 ParameterSystem::ValueParam cost);
    void clearCost();

    void updateInfo();

   private:
    // Level
    ParameterSystem::BaseState mLevel;
    ParameterSystem::ParameterSubscriptionPtr mLevelSub;

    // Cost
    std::unique_ptr<Cost> mCost;
    ParameterSystem::ParameterSubscriptionPtr mCostSub;
};

typedef std::shared_ptr<Buyable> BuyablePtr;

// Flexible upgrade
class Upgrade : public UpgradeBase {
   public:
   public:
    typedef std::function<Number(const Number&)> ValueFunc;
    typedef std::function<bool(const Number&)> StateFunc;

    Upgrade(
        ParameterSystem::BaseValue level, unsigned int maxLevel,
        std::function<void(const Number&)> onLevel = [](const Number&) {});
    Upgrade(
        ParameterSystem::BaseValue level, ParameterSystem::ValueParam maxLevel,
        std::function<void(const Number&)> onLevel = [](const Number&) {});

    Status getStatus();
    void buy();

    void setCost(ParameterSystem::BaseValue money,
                 ParameterSystem::ValueParam cost);
    void setCost(ParameterSystem::BaseValue money,
                 ParameterSystem::NodeValue cost,
                 std::function<Number(const Number&)> costFunc);
    void clearCost();

    void setEffect(
        ParameterSystem::NodeValue param, ValueFunc func,
        std::function<TextUpdateData(const Number&)> effectFunc = nullptr);
    void setEffect(ParameterSystem::NodeState param, StateFunc func,
                   std::function<TextUpdateData(bool)> effectFunc = nullptr);
    void setEffects(
        std::initializer_list<std::pair<ParameterSystem::NodeValue, ValueFunc>>
            values,
        std::initializer_list<std::pair<ParameterSystem::NodeState, StateFunc>>
            states,
        std::function<TextUpdateData()> func = nullptr);
    void clearEffects();

    void updateInfo();

   private:
    // Level
    ParameterSystem::BaseValue mLevel;
    unsigned int mMaxLevel;
    ParameterSystem::ParameterSubscriptionPtr mLevelSub, mMaxLevelSub;

    // Cost
    std::unique_ptr<Cost> mCost;
    ParameterSystem::ParameterSubscriptionPtr mCostSub;

    // Effects
    TextUpdateData mEffectText;
    std::list<ParameterSystem::ParameterSubscriptionPtr> mEffectLevelSubs;
    ParameterSystem::ParameterSubscriptionPtr mEffectSub;
};

typedef std::shared_ptr<Upgrade> UpgradePtr;

#endif
