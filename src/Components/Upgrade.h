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
#include <Systems/ParameterSystem.h>
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
    struct ParamSources {
        const static ParameterSystem::ParamBase NONE;
        const static ParameterSystem::Param<CRYSTAL> CRYSTAL_MAGIC;
    };

    struct Defaults {
        static bool CanBuy(std::shared_ptr<Upgrade> u);
        static std::string AdditiveEffect(const Number& effect);
        static std::string MultiplicativeEffect(const Number& effect);
        static std::string PercentEffect(const Number& effect);
    };

    Upgrade() = default;
    // No copying to avoid memory issues with parameter subscriptions
    Upgrade(const Upgrade& other) = delete;
    Upgrade& operator=(const Upgrade& other) = delete;

    bool mIncludeInfo = true;

    SharedTexture mImg, mDesc, mInfo;

    int getMaxLevel() const;
    int getLevel() const;
    const std::string& getEffect() const;
    bool hasEffectSrc() const;
    const ParameterSystem::ParamBase& getEffectSrc() const;
    bool hasCostSrc() const;
    const ParameterSystem::ParamBase& getCostSrc() const;
    bool hasMoneySrc() const;
    const ParameterSystem::ParamBase& getMoneySrc() const;

    Upgrade& setMaxLevel(int maxLevel);
    Upgrade& setLevel(int level);
    Upgrade& setEffect(const std::string& effect);
    template <WizardId id>
    Upgrade& setEffectSource(
        const ParameterSystem::Param<id>& param,
        std::function<std::string(const Number&)> onEffect) {
        mEffectSrc = std::make_unique<ParameterSystem::Param<id>>(param);
        mEffectSub = mEffectSrc->subscribe([this, onEffect]() {
            mEffect = onEffect(mEffectSrc->get());
            updateInfo();
        });
        return *this;
    }
    template <WizardId id, WizardType<id> key>
    Upgrade& setEffectSource(
        std::function<std::string(const Number&)> onEffect) {
        setEffectSource(ParameterSystem::Param<id>(key), onEffect);
        return *this;
    }
    Upgrade& clearEffectSource();
    template <WizardId id>
    Upgrade& setCostSource(const ParameterSystem::Param<id>& param) {
        mCostSrc = std::make_unique<ParameterSystem::Param<id>>(param);
        mCostSub = mCostSrc->subscribe(std::bind(&Upgrade::updateInfo, this));
        return *this;
    }
    template <WizardId id, WizardType<id> key>
    Upgrade& setCostSource() {
        setCostSource(ParameterSystem::Param<id>(key));
        return *this;
    }
    Upgrade& clearCostSource();
    template <WizardId id>
    Upgrade& setMoneySource(const ParameterSystem::Param<id>& param) {
        mMoneySrc = std::make_unique<ParameterSystem::Param<id>>(param);
        return *this;
    }
    template <WizardId id, WizardType<id> key>
    Upgrade& setMoneySource() {
        setMoneySource(ParameterSystem::Param<id>(key));
        return *this;
    }
    Upgrade& clearMoneySource();
    Upgrade& setImg(std::string img);
    Upgrade& setDescription(std::string desc);

    void drawDescription(TextureBuilder tex, SDL_FPoint offset = {0, 0}) const;

    std::string getInfo() const;

    void updateInfo();

    static SharedTexture createDescription(std::string text);

    const static SDL_Color DESC_BKGRND;
    const static FontData DESC_FONT;

   private:
    // maxLevel < 0: Can buy infinitely
    // maxLevel = 0: Can't buy
    // maxLevel > 0: Can by maxLevel times
    int mMaxLevel = 0;
    int mLevel = 0;
    std::string mEffect = "";

    std::unique_ptr<ParameterSystem::ParamBase> mCostSrc, mMoneySrc, mEffectSrc;
    ParameterSystem::ParameterObservable::SubscriptionPtr mCostSub, mEffectSub;
};

typedef std::shared_ptr<Upgrade> UpgradePtr;

// For managing multiple upgrades
typedef Observable<void(UpgradePtr), bool(UpgradePtr), std::shared_ptr<Upgrade>>
    UpgradeListBase;
class UpgradeList : public UpgradeListBase {
   public:
    enum : size_t { ON_LEVEL = 0, CAN_BUY, DATA };
    enum UpgradeStatus : uint8_t {
        BOUGHT = 0,
        BUYABLE,
        CANT_BUY,
        NOT_BUYABLE,
    };

    SubscriptionPtr subscribe(std::function<void(UpgradePtr)> func,
                              UpgradePtr up);
    SubscriptionPtr subscribe(UpgradePtr up);

    void onSubscribe(SubscriptionPtr sub);

    int size() const;

    void onClick(SDL_Point mouse);
    RenderObservable::SubscriptionPtr onHover(SDL_Point mouse,
                                              SDL_Point relMouse);

    void draw(TextureBuilder tex, float scroll, SDL_Point offset = {0, 0});

    // Static functions
    static UpgradePtr Get(SubscriptionPtr sub);

   private:
    UpgradeStatus getSubStatus(SubscriptionPtr sub);
    void onSubClick(SubscriptionPtr sub);

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
