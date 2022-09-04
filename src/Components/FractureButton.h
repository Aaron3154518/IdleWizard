#ifndef FRACTURE_BUTTON_H
#define FRACTURE_BUTTON_H

#include <Components/Upgrade.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/HoverService.h>
#include <ServiceSystem/EventServices/MouseService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem.h>
#include <Wizards/Definitions/CrystalDefs.h>
#include <Wizards/Money.h>
#include <Wizards/WizardIds.h>

class FractureButton : public Component, private Display {
   public:
    FractureButton();

    void setHidden(bool hidden);

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onClick(Event::MouseButton b, bool clicked);
    void onMouseEnter();
    void onMouseLeave();
    void onCrystalPos(const Rect& r);

    RenderObservable::SubscriptionPtr mRenderSub, mDescRenderSub;
    MouseObservable::SubscriptionPtr mMouseSub;
    HoverObservable::SubscriptionPtr mHoverSub;
    WizardSystem::WizardPosObservable::IdSubscriptionPtr mCrysPosSub;

    UIComponentPtr mPos;
};

#endif
