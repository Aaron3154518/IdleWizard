#include <RenderSystem/RenderSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>

#include <memory>
#include <unordered_map>

class UpgradeBase {
   public:
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
    Status getStatus();

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

class Toggle : public UpgradeBase {
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

class Upgrade : public UpgradeBase {
   private:
    struct Cost {
        Cost(ParameterSystem::BaseValue level, ParameterSystem::NodeValue cost,
             ParameterSystem::BaseValue money,
             std::function<Number(const Number&)> costFunc);

        ParameterSystem::NodeValue mCost;
        ParameterSystem::BaseValue mMoney;
        ParameterSystem::ParameterSubscriptionPtr mCostSub;
    };

    struct Effect {
        std::list<ParameterSystem::ValueParam> mValueParams;
        std::list<ParameterSystem::StateParam> mStateParams;
    };

   public:
    Upgrade(ParameterSystem::BaseValue level, int maxLevel);
    Upgrade(ParameterSystem::BaseValue level,
            ParameterSystem::ValueParam maxLevel);

    void setCost(ParameterSystem::NodeValue cost,
                 ParameterSystem::BaseValue money,
                 std::function<Number(const Number&)> costFunc);
    void clearCost();

   private:
    // Level
    ParameterSystem::BaseValue mLevel;
    int mMaxLevel;
    ParameterSystem::ParameterSubscriptionPtr mMaxLevelSub;

    // Cost
    std::unique_ptr<Cost> mCost;

    // Effects
};
