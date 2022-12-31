#ifndef CHARGE_BUTTON_H
#define CHARGE_BUTTON_H

#include <Components/Upgrade.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/HoverService.h>
#include <ServiceSystem/EventServices/MouseService.h>
#include <Systems/IconSystem/IconSystem.h>
#include <Systems/IconSystem/MoneyIcons.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Wizards/Crystal/CrystalConstants.h>
#include <Wizards/RobotWizard/RobotWizardConstants.h>
#include <Wizards/WizardIds.h>

#include <memory>

namespace RobotWizard {
class ChargeButton : public Component, private Display {
   public:
    ChargeButton();

    bool isHidden() const;
    void setHidden(bool hidden);

    void onRender(SDL_Renderer* r);

   private:
    void init();

    void onClick(Event::MouseButton b, bool clicked);
    void onMouseEnter();
    void onMouseLeave();
    void onRobotPos(const Rect& r);

    Number getGainAmnt();

    RenderObservable::SubscriptionPtr mDescRenderSub;
    MouseObservable::SubscriptionPtr mMouseSub;
    HoverObservable::SubscriptionPtr mHoverSub;
    WizardSystem::WizardPosObservable::IdSubscriptionPtr mRobotPosSub;

    UIComponentPtr mPos;
};

typedef std::unique_ptr<ChargeButton> ChargeButtonPtr;
}  // namespace RobotWizard

#endif
