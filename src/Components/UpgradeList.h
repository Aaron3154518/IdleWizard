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

// For managing multiple upgrades
typedef Observable<UpgradeBasePtr> UpgradeListBase;

class UpgradeList : public UpgradeListBase {
   public:
    enum : size_t { DATA };

    int size() const;

    void onClick(SDL_Point mouse);
    RenderObservable::SubscriptionPtr onHover(SDL_Point mouse,
                                              SDL_Point relMouse);

    void draw(TextureBuilder tex, float scroll, SDL_Point offset = {0, 0});

    std::unordered_set<void*> getActive() const;

    Number buyAll(ParameterSystem::BaseValue money, Number max = -1);
    void buyAllFree(ParameterSystem::BaseValue money);

   private:
    void computeRects();

    int mCount;
    double mScroll;
    Rect mRect;
    std::vector<std::pair<Rect, SubscriptionWPtr>> mBackRects, mFrontRects;
    std::vector<std::pair<bool, int>> mIdxMap;
};
typedef std::shared_ptr<UpgradeList> UpgradeListPtr;

// For handling wizard upgrades
typedef TargetSystem::TargetDataObservable<WizardId, UpgradeListPtr>
    WizardUpgradesObservable;

const UpgradeListPtr& GetWizardUpgrades(WizardId id);

std::shared_ptr<WizardUpgradesObservable> GetWizardUpgradesObservable();

// For setting current UpgradeObservable
class UpgradeListObservable : public ForwardObservable<void(UpgradeListPtr)> {};

class UpgradeService
    : public Service<UpgradeListObservable, WizardUpgradesObservable> {};

// For displaying current UpgradeObservable
class UpgradeScroller : public Component {
   public:
    UpgradeScroller();

    const static SDL_Color BKGRND;
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
