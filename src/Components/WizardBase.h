#ifndef WIZARD_BASE_H
#define WIZARD_BASE_H

#include <Components/UpgradeList.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/EventService.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/ParameterSystem/ParameterObservable.h>
#include <Systems/TargetSystem.h>
#include <Systems/TimeSystem.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Wizards/Definitions/Definitions.h>
#include <Wizards/WizardIds.h>

#include <memory>
#include <random>
#include <unordered_map>

class WizardBase : public Component {
   public:
    virtual ~WizardBase();

    const WizardId mId;

    const static AnimationData STAR_IMG;
    const static Rect IMG_RECT;
    const static FontData FONT;

   protected:
    WizardBase(WizardId id);

    virtual void init();
    virtual void setSubscriptions();
    virtual void setUpgrades();
    virtual void setParamTriggers();

    virtual void onResize(EventServices::ResizeData data);
    virtual void onRender(SDL_Renderer* r);
    virtual void onClick(Event::MouseButton b, bool clicked);
    bool onStarTimer(Timer& t);
    virtual void onHide(bool hide);

    virtual void showUpgrades();

    virtual void setPos(float x, float y);

    void showStar();

    void attachSubToVisibility(SubscriptionBaseWPtr wSub);
    void detachSubFromVisibility(SubscriptionBasePtr ub);

    bool mHidden = false, mShowStar = false;

    RenderAnimation mImg, mStar;

    UIComponentPtr mPos;
    EventServices::DragComponentPtr mDrag;
    EventServices::ResizeObservable::SubscriptionPtr mResizeSub;
    RenderObservable::SubscriptionPtr mRenderSub;
    EventServices::MouseObservable::SubscriptionPtr mMouseSub;
    EventServices::DragObservable::SubscriptionPtr mDragSub;
    TimerObservable::SubscriptionPtr mStarTimerSub;
    TimeSystem::TimerObservable::SubscriptionPtr mStarAnimSub;
    WizardSystem::HideObservable::IdSubscriptionPtr mHideSub;
    std::list<ParameterSystem::ParameterSubscriptionPtr> mParamSubs;

    UpgradeListPtr mUpgrades = std::make_shared<UpgradeList>();
    UpgradeActiveList mActiveUps;

    std::mt19937 gen = std::mt19937(rand());
    std::uniform_real_distribution<float> rDist;

   private:
    std::list<SubscriptionBaseWPtr> mVisibilitySubs;
};

#endif
