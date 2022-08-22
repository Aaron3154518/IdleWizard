#ifndef EMPTY_H
#define EMPTY_H

#include <RenderSystem/RenderSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>

#include <list>
#include <memory>

class UpgradeBase {
   public:
    struct Defaults {
        const static ParameterSystem::BaseValue CRYSTAL_MAGIC, CRYSTAL_SHARDS;

        static std::string AdditiveEffect(const Number& effect);
        static std::string MultiplicativeEffect(const Number& effect);
        static std::string PercentEffect(const Number& effect);
    };

    enum Status : uint8_t {
        BOUGHT = 0,
        CAN_BUY,
        CANT_BUY,
        NOT_BUYABLE,
    };

    virtual ~UpgradeBase() = default;

    virtual Status getStatus();
    virtual void buy();

    void setImage(const std::string& file);
    void setDescription(const std::string& desc);
    void setInfo(const std::string& info);

    void drawIcon(TextureBuilder& tex, const Rect& r);
    void drawDescription(TextureBuilder tex, SDL_FPoint offset = {0, 0});

    static SharedTexture createDescription(std::string text);

    const static SDL_Color DESC_BKGRND;
    const static FontData DESC_FONT;

   protected:
    std::string mInfoStr = "";
    bool mUpdateInfo = false;

   private:
    SharedTexture mImg, mDesc, mInfo;
};

typedef std::shared_ptr<UpgradeBase> UpgradeBasePtr;

class Display : public UpgradeBase {
   public:
    virtual ~Display() = default;

    virtual Status getStatus();

    void setEffect(ParameterSystem::ValueParam param,
                   std::function<std::string(const Number&)> func);
    void setEffect(ParameterSystem::StateParam param,
                   std::function<std::string(bool)> func);
    void setEffects(
        const std::initializer_list<ParameterSystem::ValueParam>& valueParams,
        const std::initializer_list<ParameterSystem::StateParam>& stateParams,
        std::function<std::string()> func);

   private:
    ParameterSystem::ParameterSubscriptionPtr mEffectSub;
};

typedef std::shared_ptr<Display> DisplayPtr;

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

class Upgrade : public UpgradeBase {
   public:
    struct Cost {
       public:
        Cost(ParameterSystem::BaseValue money, ParameterSystem::NodeValue cost);
        Cost(ParameterSystem::BaseValue level, ParameterSystem::BaseValue money,
             ParameterSystem::NodeValue cost,
             std::function<Number(const Number&)> costFunc);

        const Number& getCost() const;
        const Number& getMoney() const;
        bool canBuy() const;
        void buy() const;

        ParameterSystem::ParameterSubscriptionPtr subscribe(
            std::function<void()> func) const;

       private:
        ParameterSystem::NodeValue mCost;
        ParameterSystem::BaseValue mMoney;
        ParameterSystem::ParameterSubscriptionPtr mCostSub;
    };

    struct Effects {
       public:
        typedef std::function<Number(const Number&)> ValueFunc;
        typedef std::function<bool(const Number&)> StateFunc;
        typedef std::function<std::string()> EffectFunc;

        Effects();
        Effects(EffectFunc func);

        Effects& addEffect(ParameterSystem::NodeValue param, ValueFunc func);
        Effects& addEffect(ParameterSystem::NodeValue param, ValueFunc valFunc,
                           std::function<std::string(const Number&)> effFunc);
        Effects& addEffect(ParameterSystem::NodeState param, StateFunc func);
        Effects& addEffect(ParameterSystem::NodeState param,
                           StateFunc stateFunc,
                           std::function<std::string(bool)> effFunc);

        std::list<ParameterSystem::ParameterSubscriptionPtr> subscribeToLevel(
            ParameterSystem::BaseValue level) const;
        ParameterSystem::ParameterSubscriptionPtr subscribeToEffects(
            std::function<void(const std::string&)> func) const;

       private:
        EffectFunc mGetEffect = nullptr;
        std::list<std::pair<ParameterSystem::NodeValue, ValueFunc>>
            mValueParams;
        std::list<std::pair<ParameterSystem::NodeState, StateFunc>>
            mStateParams;
    };

   public:
    Upgrade(
        ParameterSystem::BaseValue level, unsigned int maxLevel,
        std::function<void(const Number&)> onLevel = [](const Number&) {});
    Upgrade(
        ParameterSystem::BaseValue level, ParameterSystem::ValueParam maxLevel,
        std::function<void(const Number&)> onLevel = [](const Number&) {});

    Status getStatus();
    void buy();

    void setCost(ParameterSystem::BaseValue money,
                 ParameterSystem::NodeValue cost);
    void setCost(ParameterSystem::BaseValue money,
                 ParameterSystem::NodeValue cost,
                 std::function<Number(const Number&)> costFunc);
    void clearCost();

    void setEffects(const Effects& effects);
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
    std::string mEffectStr = "";
    std::list<ParameterSystem::ParameterSubscriptionPtr> mEffectLevelSubs;
    ParameterSystem::ParameterSubscriptionPtr mEffectSub;
};

typedef std::shared_ptr<Upgrade> UpgradePtr;

#endif
