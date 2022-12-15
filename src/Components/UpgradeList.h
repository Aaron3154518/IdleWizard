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

    UpgradeActiveList getSnapshot() const;

    bool canBuyOne(ParameterSystem::BaseValue money, Number max = -1);
    Number upgradeAll(ParameterSystem::BaseValue money, Number max = -1);
    void maxAll(ParameterSystem::BaseValue money);
};
typedef std::shared_ptr<UpgradeList> UpgradeListPtr;

// For handling wizard upgrades
typedef TargetSystem::TargetDataObservable<WizardId, UpgradeListPtr>
    WizardUpgradesObservable;

const UpgradeListPtr& GetWizardUpgrades(WizardId id);

std::shared_ptr<WizardUpgradesObservable> GetWizardUpgradesObservable();

// For setting current UpgradeObservable
class UpgradeListObservable
    : public ForwardObservable<void(UpgradeListPtr),
                               void(UpgradeListPtr,
                                    ParameterSystem::ValueParam)> {};

class UpgradeService
    : public Service<UpgradeListObservable, WizardUpgradesObservable> {};

namespace UpgradeRendering {}  // namespace UpgradeRendering

// Handles rendering upgrade lists
class UpgradeRenderer {
   public:
    virtual void onClick(SDL_Point mouse);
    virtual RenderObservable::SubscriptionPtr onHover(SDL_Point mouse,
                                                      SDL_Point relMouse);

    virtual void draw(TextureBuilder tex, float scroll,
                      SDL_Point offset = {0, 0});

    virtual float minScroll() const;
    virtual float maxScroll() const;

   private:
};

typedef std::unique_ptr<UpgradeRenderer> UpgradeRendererPtr;

// Renders upgrades as elliptical scroller
class UpgradeScroller : public UpgradeRenderer {
   public:
    UpgradeScroller(UpgradeListPtr upgrades);

    void onClick(SDL_Point mouse);
    RenderObservable::SubscriptionPtr onHover(SDL_Point mouse,
                                              SDL_Point relMouse);

    void draw(TextureBuilder tex, float scroll, SDL_Point offset = {0, 0});

    float maxScroll() const;

   private:
    void computeRects();

    const UpgradeListPtr mUpgrades;

    int mCount = -1;
    float mScroll = 0;
    SDL_Point mDim = {0, 0};
    std::vector<std::pair<Rect, UpgradeList::SubscriptionWPtr>> mBackRects,
        mFrontRects;
    std::vector<std::pair<bool, int>> mIdxMap;
};

// Renders upgrades as progressbar
class UpgradeProgressBar : public UpgradeRenderer {
   public:
    typedef std::pair<float, UpgradeList::SubscriptionWPtr> UpgradeCost;

    UpgradeProgressBar(UpgradeListPtr upgrades,
                       ParameterSystem::ValueParam val);

    RenderObservable::SubscriptionPtr onHover(SDL_Point mouse,
                                              SDL_Point relMouse);

    void draw(TextureBuilder tex, float scroll, SDL_Point offset = {0, 0});

    float maxScroll() const;

   private:
    static float toValue(const Number& val);

    const UpgradeListPtr mUpgrades;
    const ParameterSystem::ValueParam mValParam;

    float mScrollMargin = 0;
    Rect mBounds;

    std::vector<UpgradeCost> mCostVals;
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
    void onSetUpgrades(UpgradeListPtr list);
    void onSetUpgrades(UpgradeListPtr list, ParameterSystem::ValueParam val);

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
    UpgradeListObservable::SubscriptionPtr mUpgradeSub;
};

#endif
