#ifndef UPGRADE_H
#define UPGRADE_H

#include <RenderSystem/RenderSystem.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <ServiceSystem/MouseService/DragService.h>
#include <ServiceSystem/MouseService/MouseService.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Utils/Colors.h>
#include <Utils/Event.h>
#include <Utils/Number.h>
#include <Utils/Rect.h>

#include <cmath>
#include <forward_list>
#include <vector>

#include "WizardIds.h"

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

    void scroll(float dScroll);
    float maxScroll() const;
    void draw();

    std::vector<int> mUpgrades = {1, 1, 2, 3, 5, 8, 13, 21, 35, 56};

    float mScroll = 0, mScrollV = 0;
    RectData mBkgrnd;
    TextureBuilder mTex;
    RenderData mTexData;

    DragComponentPtr mDragComp;

    UpdateObservable::SubscriptionPtr mUpdateSub;
    RenderObservable::SubscriptionPtr mRenderSub;
    MouseObservable::SubscriptionPtr mMouseSub;
    DragObservable::SubscriptionPtr mDragSub;
};

#endif
