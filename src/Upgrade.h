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
#include <Utils/Colors.h>
#include <Utils/Event.h>
#include <Utils/Number.h>
#include <Utils/Rect.h>

#include <cmath>
#include <forward_list>
#include <functional>
#include <vector>

#include "WizardIds.h"

typedef ReplyObservable<RenderData()> RenderReply;

struct Upgrade {
    // maxLevel < 0: Can buy infinitely
    // maxLevel = 0: Can't buy
    // maxLevel > 0: Can by maxLevel times
    int mMaxLevel;
    int mLevel = 0;
    Number mCost;
    std::string mEffect;
    bool mIncludeInfo = true;

    SharedTexture mImg, mDesc, mInfo;

    void setImg(std::string img);

    void setDescription(std::string desc);

    void drawDescription(TextureBuilder tex, SDL_Point offset = {0, 0}) const;

    void updateInfo();

    static SharedTexture createDescription(std::string text);

    const static SDL_Color DESC_BKGRND;
    const static FontData DESC_FONT;
};
typedef std::shared_ptr<Upgrade> UpgradePtr;

// For managing multiple upgrades
typedef Observable<void(Upgrade&), Number(Upgrade&), bool(Upgrade&), Upgrade>
    UpgradeListBase;
class UpgradeList : public UpgradeListBase {
   public:
    enum : size_t { ON_LEVEL = 0, GET_COST, CAN_BUY, DATA };
    enum UpgradeStatus : uint8_t {
        BOUGHT = 0,
        BUYABLE,
        CANT_BUY,
    };

    void onSubscribe(SubscriptionPtr sub);

    int count() const;

    void onClick(SDL_Point mouse);
    RenderObservable::SubscriptionPtr onHover(SDL_Point mouse,
                                              SDL_Point relMouse);

    void draw(TextureBuilder tex, float scroll);

   private:
    UpgradeStatus getSubStatus(SubscriptionPtr sub);
    void onSubClick(SubscriptionPtr sub);

    void computeRects();

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
