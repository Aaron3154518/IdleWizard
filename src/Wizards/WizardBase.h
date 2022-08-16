#ifndef WIZARD_BASE_H
#define WIZARD_BASE_H

#include <Components/Fireball.h>
#include <Components/Upgrade.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/DragService.h>
#include <ServiceSystem/EventServices/MouseService.h>
#include <ServiceSystem/EventServices/ResizeService.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/ParameterSystem.h>
#include <Systems/TargetSystem.h>
#include <Wizards/WizardIds.h>

#include <memory>
#include <random>

class WizardBase : public Component {
   public:
    typedef ForwardObservable<void(WizardId, bool)> HideObservableBase;
    class HideObservable : public HideObservableBase {
       public:
        void next(WizardId id, bool hide);

        bool isHidden(WizardId id) const;

       private:
        std::unordered_map<WizardId, bool> mHidden;
    };

    static bool Hidden(WizardId id);

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
    virtual void onHide(WizardId id, bool hide);

    virtual void setPos(float x, float y);

    void setImage(const std::string& img);

    void attachSubToVisibility(SubscriptionBaseWPtr wSub);
    void detachSubFromVisibility(SubscriptionBasePtr ub);

    bool mHidden = false;

    RenderData mImg;

    UIComponentPtr mPos;
    DragComponentPtr mDrag;
    ResizeObservable::SubscriptionPtr mResizeSub;
    RenderObservable::SubscriptionPtr mRenderSub;
    MouseObservable::SubscriptionPtr mMouseSub;
    DragObservable::SubscriptionPtr mDragSub;
    HideObservable::SubscriptionPtr mHideSub;
    std::list<ParameterSystem::ParameterObservable::SubscriptionPtr> mParamSubs;

    UpgradeListPtr mUpgrades = std::make_shared<UpgradeList>();

    std::mt19937 gen = std::mt19937(rand());
    std::uniform_real_distribution<> rDist;

   private:
    std::list<SubscriptionBaseWPtr> mVisibilitySubs;
};

class WizardService : public Service<WizardBase::HideObservable> {};

#endif
