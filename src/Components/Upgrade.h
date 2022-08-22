#ifndef UPGRADE_H
#define UPGRADE_H

#include <RenderSystem/RenderSystem.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <ServiceSystem/EventServices/DragService.h>
#include <ServiceSystem/EventServices/HoverService.h>
#include <ServiceSystem/EventServices/MouseService.h>
#include <ServiceSystem/EventServices/ResizeService.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/ParameterObservable.h>
#include <Utils/Colors.h>
#include <Utils/Event.h>
#include <Utils/Number.h>
#include <Utils/Rect.h>
#include <Wizards/WizardIds.h>

#include <cmath>
#include <forward_list>
#include <functional>
#include <vector>

class Upgrade {
   public:
    enum Status : uint8_t {
        BOUGHT = 0,
        BUYABLE,
        CANT_BUY,
        NOT_BUYABLE,
    };

    struct ValueUpdate {
        ParameterSystem::NodeValue node;
        std::function<Number(const Number&)> func;
    };

    struct StateUpdate {
        ParameterSystem::NodeState node;
        std::function<bool(const Number&)> func;
    };

    struct Defaults {
        const static ParameterSystem::BaseValue CRYSTAL_MAGIC, CRYSTAL_SHARDS,
            CATALYST_MAGIC;

        static bool CanBuy(std::shared_ptr<Upgrade> u);
        static std::string AdditiveEffect(const Number& effect);
        static std::string MultiplicativeEffect(const Number& effect);
        static std::string PercentEffect(const Number& effect);
    };

    Upgrade(ParameterSystem::BaseValue lvl);
    // No copying to avoid memory issues with parameter subscriptions
    Upgrade(const Upgrade& other) = delete;
    Upgrade& operator=(const Upgrade& other) = delete;

    bool mIncludeInfo = true;

    Upgrade& setMaxLevel(int maxLevel);
    Upgrade& setOnLevel(std::function<void(const Number&)> func);
    Upgrade& setMoney(ParameterSystem::BaseValue param);
    Upgrade& setCost(ParameterSystem::NodeValue param,
                     std::function<Number(const Number&)> func);
    Upgrade& setEffectStr(const std::string& str);
    Upgrade& setEffect(
        ValueUpdate val,
        std::function<std::string(const Number&)> getEffectString);
    Upgrade& setEffect(StateUpdate state,
                       std::function<std::string(bool)> getEffectString);
    Upgrade& setEffects(const std::initializer_list<ValueUpdate>& values,
                        const std::initializer_list<StateUpdate>& states,
                        std::function<std::string()> getEffectString);

    Upgrade& setImg(std::string img);
    Upgrade& setDescription(std::string desc);

    Status getStatus() const;
    void buy();

    void drawIcon(TextureBuilder& tex, const Rect& r);
    void drawDescription(TextureBuilder tex, SDL_FPoint offset = {0, 0});

    std::string getInfo() const;

    static SharedTexture createDescription(std::string text);

    const static SDL_Color DESC_BKGRND;
    const static FontData DESC_FONT;

   protected:
    Upgrade& addEffect(ValueUpdate val);
    Upgrade& addEffect(StateUpdate state);
    Upgrade& clearEffects();

    // maxLevel < 0: Can buy infinitely
    // maxLevel = 0: Can't buy
    // maxLevel > 0: Can by maxLevel times
    int mMaxLevel = 0;

    std::string mEffect = "";
    bool mUpdateInfo = false;
    SharedTexture mImg, mDesc, mInfo;

    ParameterSystem::NodeValuePtr mCostSrc;
    ParameterSystem::BaseValuePtr mLevelSrc, mMoneySrc;
    std::list<ParameterSystem::NodeValue> mValueEffectSrcs;
    std::list<ParameterSystem::NodeState> mStateEffectSrcs;

    std::list<ParameterSystem::ParameterSubscriptionPtr> mEffectSubs;
    ParameterSystem::ParameterSubscriptionPtr mLevelSub, mCostSub, mEffectSub;
};

typedef std::shared_ptr<Upgrade> UpgradePtr;

class TileUpgrade : public Upgrade {
   public:
    TileUpgrade() = default;

    Upgrade& setMoney(ParameterSystem::BaseValue param);
    Upgrade& setCost(ParameterSystem::NodeValue param,
                     std::function<Number(const Number&)> func);

    Upgrade& setEffect(
        ParameterSystem::NodeValue val,
        std::function<std::string(const Number&)> getEffectString);
    Upgrade& setEffect(ParameterSystem::NodeState state,
                       std::function<std::string(bool)> getEffectString);
    Upgrade& setEffects(
        const std::initializer_list<ParameterSystem::NodeValue>& values,
        const std::initializer_list<ParameterSystem::NodeState>& states,
        std::function<std::string()> getEffectString);
};

typedef std::shared_ptr<TileUpgrade> TileUpgradePtr;

// For managing multiple upgrades
typedef Observable<UpgradePtr> UpgradeListBase;
class UpgradeList : public UpgradeListBase {
   public:
    enum : size_t { DATA };

    int size() const;

    void onClick(SDL_Point mouse);
    RenderObservable::SubscriptionPtr onHover(SDL_Point mouse,
                                              SDL_Point relMouse);

    void draw(TextureBuilder tex, float scroll, SDL_Point offset = {0, 0});

    // Static functions
    static UpgradePtr Get(SubscriptionPtr sub);

   private:
    void computeRects();

    int mCount;
    double mScroll;
    Rect mRect;
    std::vector<std::pair<Rect, SubscriptionWPtr>> mBackRects, mFrontRects;
};
typedef std::shared_ptr<UpgradeList> UpgradeListPtr;

// For setting current UpgradeObservable
class UpgradeListObservable : public ForwardObservable<void(UpgradeListPtr)> {};

class UpgradeService : public Service<UpgradeListObservable> {};

// For displaying current UpgradeObservable
class UpgradeScroller : public Component {
   public:
    UpgradeScroller();

    const static SDL_Color BGKRND;
    const static Rect RECT;

   private:
    void init();

    void onResize(ResizeData data);
    void onUpdate(Time dt);
    void onRender(SDL_Renderer* r);
    void onClick(Event::MouseButton b, bool clicked);
    void onDrag(int mouseX, int mouseY, float mouseDx, float mouseDy);
    void onDragStart();
    void onHover(SDL_Point mouse);
    void onMouseLeave();
    void onSetUpgrades(UpgradeListPtr list);

    void scroll(float dScroll);
    float maxScroll() const;

    float mScroll = 0, mScrollV = 0;
    TextureBuilder mTex;
    RenderData mTexData;

    UIComponentPtr mPos;
    DragComponentPtr mDrag;

    UpgradeListPtr mUpgrades;

    ResizeObservable::SubscriptionPtr mResizeSub;
    UpdateObservable::SubscriptionPtr mUpdateSub;
    RenderObservable::SubscriptionPtr mRenderSub, mUpDescRenderSub;
    MouseObservable::SubscriptionPtr mMouseSub;
    DragObservable::SubscriptionPtr mDragSub;
    HoverObservable::SubscriptionPtr mHoverSub;
    UpgradeListObservable::SubscriptionPtr mUpgradeSub;
};

#endif
