#ifndef UPGRADE_LIST_H
#define UPGRADE_LIST_H

#include <Components/Upgrade.h>
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
#include <Utils/Colors.h>
#include <Utils/Event.h>
#include <Utils/Number.h>
#include <Utils/Rect.h>
#include <Wizards/WizardIds.h>

#include <cmath>
#include <functional>
#include <unordered_set>
#include <vector>

typedef std::unordered_map<void*, UpgradeSnapshot> UpgradeActiveList;

// For managing multiple upgrades
typedef Observable<UpgradeBasePtr> UpgradeListBase;

class UpgradeList : public UpgradeListBase {
    friend class UpgradeScroller;
    friend class UpgradeProgressBar;

   public:
    enum : size_t { DATA };

    int size() const;

    float getScroll() const;
    void setScroll(float scroll);
    bool isOpen() const;
    void setOpen(bool open);

    UpgradeActiveList getSnapshot() const;

    bool canBuyOne(ParameterSystem::BaseValue money, Number max = -1);
    Number upgradeAll(ParameterSystem::BaseValue money, Number max = -1);
    void maxAll(ParameterSystem::BaseValue money);

   private:
    float mScroll = 0;
    bool mOpen = false;
};
typedef std::shared_ptr<UpgradeList> UpgradeListPtr;

// For handling wizard upgrades
typedef TargetSystem::TargetDataObservable<WizardId, UpgradeListPtr>
    WizardUpgradesObservable;

const UpgradeListPtr& GetWizardUpgrades(WizardId id);

std::shared_ptr<WizardUpgradesObservable> GetWizardUpgradesObservable();

// For setting current UpgradeObservable
class UpgradeListObservable
    : public ForwardObservable<void(UpgradeListPtr, RenderTextureCPtr),
                               void(UpgradeListPtr, ParameterSystem::BaseValue,
                                    ParameterSystem::ValueParam)> {};

class UpgradeService
    : public Service<UpgradeListObservable, WizardUpgradesObservable> {};

// Handles rendering upgrade lists
class UpgradeRenderer {
   public:
    UpgradeRenderer(UpgradeListPtr upgrades);
    virtual ~UpgradeRenderer() = default;

    virtual void onClick(SDL_Point mouse);
    virtual RenderObservable::SubscriptionPtr onHover(SDL_Point mouse,
                                                      SDL_Point relMouse);

    virtual void draw(TextureBuilder tex);
    virtual void update(Rect pos, float scroll);

    virtual float minScroll() const;
    virtual float maxScroll() const;

    void open(Rect pos, float scroll);
    void close(float scroll);

    float getScroll() const;

   protected:
    const UpgradeListPtr mUpgrades;

    float mMinScroll = 0, mMaxScroll = 0;
};

typedef std::unique_ptr<UpgradeRenderer> UpgradeRendererPtr;

// Renders upgrades as elliptical scroller
class UpgradeScroller : public UpgradeRenderer {
   public:
    UpgradeScroller(UpgradeListPtr upgrades, RenderTextureCPtr img);

    void onClick(SDL_Point mouse);
    RenderObservable::SubscriptionPtr onHover(SDL_Point mouse,
                                              SDL_Point relMouse);

    void draw(TextureBuilder tex);

   private:
    void update(Rect pos, float scroll);

    RenderData mImg;

    std::vector<std::pair<Rect, UpgradeList::SubscriptionWPtr>> mBackRects,
        mFrontRects;
    std::vector<std::pair<bool, int>> mIdxMap;
};

// Renders upgrades as progressbar
class UpgradeProgressBar : public UpgradeRenderer {
   public:
    typedef std::pair<Number, UpgradeList::SubscriptionWPtr> UpgradeCost;

    const static int BUCKET_W;

    UpgradeProgressBar(UpgradeListPtr upgrades,
                       ParameterSystem::BaseValue money,
                       ParameterSystem::ValueParam val);

    RenderObservable::SubscriptionPtr onHover(SDL_Point mouse,
                                              SDL_Point relMouse);

    void draw(TextureBuilder tex);

   private:
    void update(Rect pos, float scroll);

    static float toValue(const Number& val);

    const ParameterSystem::BaseValue mMoneyParam;
    const ParameterSystem::ValueParam mValParam;

    Number mNextCost;
    int mUnlocked = 0;
    RectShape mRs;
    ProgressBar mPb;

    std::vector<std::pair<Rect, UpgradeList::SubscriptionWPtr>> mRects;
};

// Manages upgrade display
class UpgradeDisplay : public Component {
   public:
    static const SDL_Color BKGRND;
    static const Rect RECT;

    UpgradeDisplay();

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
    bool onTimer(Time& timer);
    void setUpgrades(UpgradeRendererPtr upRenderer);
    void onSetUpgrades(UpgradeListPtr list, RenderTextureCPtr img);
    void onSetUpgrades(UpgradeListPtr list, ParameterSystem::BaseValue money,
                       ParameterSystem::ValueParam val);

    void scroll(float dScroll);

    float mScroll = 0, mScrollV = 0;
    TextureBuilder mTex;
    RenderData mTexData;

    UIComponentPtr mPos;
    DragComponentPtr mDrag;

    UpgradeRendererPtr mUpRenderer;

    ResizeObservable::SubscriptionPtr mResizeSub;
    UpdateObservable::SubscriptionPtr mUpdateSub;
    RenderObservable::SubscriptionPtr mRenderSub, mUpDescRenderSub;
    MouseObservable::SubscriptionPtr mMouseSub;
    DragObservable::SubscriptionPtr mDragSub;
    HoverObservable::SubscriptionPtr mHoverSub;
    TimerObservable::SubscriptionPtr mTimerSub;
    UpgradeListObservable::SubscriptionPtr mUpgradeSub;
};

#endif
