#ifndef GLOB_H
#define GLOB_H

#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderSystem.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <SDL.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/ResizeService.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/TimeSystem.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Utils/AnimationData.h>
#include <Utils/Time.h>
#include <Wizards/PoisonWizard/PoisonWizardConstants.h>
#include <Wizards/WizardIds.h>

#include <memory>

namespace PoisonWizard {
class Glob : public Component {
    friend class GlobObservable;

   public:
    typedef Observable<void(), UIComponentPtr> HitObservableBase;
    class HitObservable : public HitObservableBase {
       public:
        enum : uint8_t { FUNC = 0, DATA };

        Rect next(const Rect& rect);
    };

    static std::shared_ptr<HitObservable> GetHitObservable();

   public:
    Glob(SDL_FPoint c, SDL_FPoint v);

    bool dead() const;

    void setPos(float x, float y);

    const static Rect IMG_RECT;

   protected:
    void init();

    void onUpdate(Time dt);
    void onResize(ResizeData data);
    void onRender(SDL_Renderer* renderer);

    void onDeath();

    bool mDead = false;
    int mBncCnt = 3;
    SDL_FPoint mV{0, 0};
    UIComponentPtr mPos;

    RenderAnimation mImg;

    ResizeObservable::SubscriptionPtr mResizeSub;
    TimerObservable::SubscriptionPtr mAnimTimerSub;
    TimeSystem::UpdateObservable::SubscriptionPtr mUpdateSub;
    RenderObservable::SubscriptionPtr mRenderSub;
};

typedef std::unique_ptr<Glob> GlobPtr;

class GlobService : public Service<Glob::HitObservable> {};
}  // namespace PoisonWizard

#endif