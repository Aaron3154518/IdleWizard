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
    template <class T>
    using UpgradeFunc = std::function<T(Upgrade&)>;

   public:
    enum Status : uint8_t {
        BOUGHT = 0,
        CAN_BUY,
        CANT_BUY,
    };

    Upgrade(int maxLvl);
    ~Upgrade() = default;

    void onClick();
    Status getStatus();

    void setOnLevel(UpgradeFunc<void> func);
    void setGetCost(UpgradeFunc<Number> func);
    void setCanBuy(UpgradeFunc<bool> func);

    void setImg(std::string img);
    void setImg(SharedTexture img);
    void setImgHandler(std::function<RenderData()> func);

    void requestImg(std::function<void(RenderData)> func);

    void setDescription(std::string desc);
    void setDescription(SharedTexture descTex);
    void setDescriptionHandler(std::function<RenderData()> func);

    void requestDescription(std::function<void(RenderData)> func);

    static SharedTexture CreateDescription(std::string text);
    static SharedTexture CreateDescription(std::string text, int level,
                                           int maxLevel, Number cost,
                                           Number effect);
    const static SDL_Color DESC_BKGRND;
    const static FontData DESC_FONT;

    // maxLevel < 0: Can buy infinitely
    // maxLevel = 0: Can't buy
    // maxLevel > 0: Can by maxLevel times
    int mLevel = 0, mMaxLevel;
    Number mCost = 0;

   private:
    UpgradeFunc<void> mOnLevel = [](Upgrade& u) {};
    UpgradeFunc<Number> mGetCost = [](Upgrade& u) { return 0; };
    UpgradeFunc<bool> mCanBuy = [](Upgrade& u) { return false; };

    RenderData mImg, mDesc;
    RenderReply mImgReply, mDescReply;
    RenderReply::RequestObservable::SubscriptionPtr mImgHandler, mDescHandler;
};

typedef std::shared_ptr<Upgrade> UpgradePtr;
typedef std::weak_ptr<Upgrade> UpgradeWPtr;
typedef Observable<void(UpgradePtr), Number(UpgradePtr), bool(UpgradePtr),
                   UpgradePtr>
    UpgradeObservableBase;

class UpgradeObservable : public UpgradeObservableBase {
   public:
    enum : size_t { ON_LEVEL = 0, GET_COST, CAN_BUY, DATA };

    void onSubscribe(SubscriptionPtr sub);

    void setScroll(double scroll);
    void setRect(Rect r);

    void click(SDL_Point mouse);
    void draw(TextureBuilder tex);

    const static SDL_Color BGKRND;

   private:
    Upgrade::Status getSubStatus(SubscriptionPtr sub);
    void onSubClick(SubscriptionPtr sub);

    void computeRects();

    double mScroll;
    Rect mRect;
    std::vector<std::pair<Rect, SubscriptionWPtr>> mBackRects, mFrontRects;
};

typedef std::vector<std::shared_ptr<Upgrade>> UpgradeList;
typedef ForwardObservable<void(const UpgradeList&)> UpgradeListObservableBase;

class UpgradeListObservable : public UpgradeListObservableBase {};

class UpgradeService : public Service<UpgradeListObservable> {};

class UpgradeScroller : public Component {
   public:
    UpgradeScroller();

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
    void onSetUpgrades(const UpgradeList& list);

    void scroll(float dScroll);
    float maxScroll() const;

    void computeRects();
    void draw();

    float mScroll = 0, mScrollV = 0;
    TextureBuilder mTex;
    RenderData mTexData;

    UIComponentPtr mPos;
    DragComponentPtr mDrag;

    UpgradeObservable mUpgrades;

    ResizeObservable::SubscriptionPtr mResizeSub;
    UpdateObservable::SubscriptionPtr mUpdateSub;
    RenderObservable::SubscriptionPtr mRenderSub, mUpDescRenderSub;
    MouseObservable::SubscriptionPtr mMouseSub;
    DragObservable::SubscriptionPtr mDragSub;
    HoverObservable::SubscriptionPtr mHoverSub;
    UpgradeListObservable::SubscriptionPtr mUpgradeSub;
};

#endif
