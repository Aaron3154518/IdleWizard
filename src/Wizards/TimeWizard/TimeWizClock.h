#ifndef TIME_WIZ_CLOCK_H
#define TIME_WIZ_CLOCK_H

#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Utils/Rect.h>
#include <Wizards/WizardIds.h>

namespace TimeWizard {
class Clock : public Component {
   public:
    Clock(Rect r);

    void setRect(const Rect& r);

    const static float SMALL_HAND_SPD, LARGE_HAND_SPD;
    const static std::string CLOCK, LARGE_HAND, SMALL_HAND;

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onUpdate(Time dt);

    float mLargeRot = 0, mSmallRot = 0;
    UIComponentPtr mPos;
    RenderData mClock, mLargeHand, mSmallHand;

    RenderObservable::SubscriptionPtr mRenderSub;
    UpdateObservable::SubscriptionPtr mUpdateSub;
};
}  // namespace TimeWizard

#endif
