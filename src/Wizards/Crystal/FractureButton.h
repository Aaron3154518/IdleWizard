#ifndef FRACTURE_BUTTON_H
#define FRACTURE_BUTTON_H

#include <Components/Upgrade.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/EventService.h>
#include <ServiceSystem/EventServices/EventService.h>
#include <Systems/IconSystem/IconSystem.h>
#include <Systems/IconSystem/MoneyIcons.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Wizards/Crystal/CrystalConstants.h>
#include <Wizards/WizardIds.h>

#include <memory>

namespace Crystal {
class FractureButton : public Component, private Display {
   public:
    FractureButton();

    bool isHidden() const;
    void setHidden(bool hidden);

    void onRender(SDL_Renderer* r);

   private:
    void init();

    void onClick(Event::MouseButton b, bool clicked);
    void onMouseEnter();
    void onMouseLeave();
    void onCrystalPos(const Rect& r);

    RenderObservable::SubscriptionPtr mDescRenderSub;
    EventServices::MouseObservable::SubscriptionPtr mMouseSub;
    EventServices::HoverObservable::SubscriptionPtr mHoverSub;
    WizardSystem::WizardPosObservable::IdSubscriptionPtr mCrysPosSub;

    UIComponentPtr mPos;
};

typedef std::unique_ptr<FractureButton> FractureButtonPtr;
}  // namespace Crystal

#endif
