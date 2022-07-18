#ifndef UPGRADE_H
#define UPGRADE_H

#include <RenderSystem/RenderSystem.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <ServiceSystem/EventServices/DragService.h>
#include <ServiceSystem/EventServices/MouseService.h>
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

struct Upgrade {
    enum Status : uint8_t {
        BOUGHT = 0,
        CAN_BUY,
        CANT_BUY,
    };

    std::function<void()> onClick = []() {};
    std::function<Status()> status = []() { return Status::CANT_BUY; };
    std::function<SharedTexture()> getImage = []() {
        return makeSharedTexture();
    };
    std::function<SharedTexture()> getDescription = []() {
        return makeSharedTexture();
    };
};

typedef std::vector<std::shared_ptr<Upgrade>> UpgradeList;
typedef Observable<const UpgradeList&, void(const UpgradeList&)>
    UpgradeObservableBase;

class UpgradeObservable : public UpgradeObservableBase {};

class UpgradeService : public Service<UpgradeObservable> {};

class UpgradeScroller : public Component {
   public:
    UpgradeScroller();

   private:
    void init();

    void onUpdate(Time dt);
    void onRender(SDL_Renderer* r);
    void onClick(Event::MouseButton b, bool clicked);
    void onDrag(int mouseX, int mouseY, float mouseDx, float mouseDy);
    void onDragStart();
    void onSetUpgrades(const UpgradeList& list);

    void scroll(float dScroll);
    float maxScroll() const;

    void computeRects();
    void draw();

    std::vector<std::weak_ptr<Upgrade>> mUpgrades;
    std::vector<std::pair<Rect, int>> mBackRects, mFrontRects;

    float mScroll = 0, mScrollV = 0;
    RectData mBkgrnd;
    TextureBuilder mTex;
    RenderData mTexData;

    DragComponentPtr mDragComp;

    UpdateObservable::SubscriptionPtr mUpdateSub;
    RenderObservable::SubscriptionPtr mRenderSub;
    MouseObservable::SubscriptionPtr mMouseSub;
    DragObservable::SubscriptionPtr mDragSub;
    UpgradeObservable::SubscriptionPtr mUpgradeSub;
};

#endif
