#ifndef CRYSTAL_H
#define CRYSTAL_H

#include <Components/FireRing.h>
#include <Components/Fireball.h>
#include <Components/Upgrade.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/MouseService.h>
#include <Systems/ParameterSystem.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>
#include <Wizards/WizardTypes.h>

#include <memory>
#include <vector>

class Crystal : public WizardBase {
   public:
    Crystal();

    const static Number T1Cost1, T1Cost2;

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onClick(Event::MouseButton b, bool clicked);
    void onHide(WizardId id, bool hide);
    void onFireballHit(const Fireball& fireball);

    void calcMagicEffect();
    void drawMagic();

    std::unique_ptr<FireRing>& createFireRing(const Number& val);

    Fireball::HitObservable::IdSubscriptionPtr mFireballSub;
    UpgradeList::SubscriptionPtr mMagicEffectDisplay, mPowWizBuy, mTimeWizBuy;

    std::vector<std::unique_ptr<FireRing>> mFireRings;

    TextRenderData mMagicText;
};

#endif
