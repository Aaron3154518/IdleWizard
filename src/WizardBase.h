#ifndef WIZARD_BASE_H
#define WIZARD_BASE_H

#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/DragService.h>
#include <ServiceSystem/EventServices/MouseService.h>
#include <ServiceSystem/EventServices/ResizeService.h>
#include <ServiceSystem/ServiceSystem.h>

#include <memory>
#include <random>

#include "Fireball.h"
#include "Upgrade.h"
#include "WizardData.h"
#include "WizardIds.h"

class WizardBase : public Component {
   public:
    virtual ~WizardBase();

    const WizardId mId;

    const static Rect IMG_RECT;
    const static std::string IMGS[];
    const static FontData FONT;

   protected:
    WizardBase(WizardId id);

    virtual void init();

    virtual void onResize(ResizeData data);
    virtual void onRender(SDL_Renderer* r);
    virtual void onClick(Event::MouseButton b, bool clicked);

    void setPos(float x, float y);

    void setImage(const std::string& img);

    RenderData mImg;

    UIComponentPtr mPos;
    DragComponentPtr mDrag;
    ResizeObservable::SubscriptionPtr mResizeSub;
    RenderObservable::SubscriptionPtr mRenderSub;
    MouseObservable::SubscriptionPtr mMouseSub;
    DragObservable::SubscriptionPtr mDragSub;
    std::list<ParameterSubscriptionPtr> mParamSubs;

    UpgradeListPtr mUpgrades = std::make_shared<UpgradeList>();

    std::mt19937 gen = std::mt19937(rand());
    std::uniform_int_distribution<> dist =
        std::uniform_int_distribution<>(1, size - 1);
    std::uniform_real_distribution<> rDist;
};

#endif
