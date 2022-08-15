#ifndef CRYSTAL_H
#define CRYSTAL_H

#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/MouseService.h>

#include <memory>
#include <vector>

#include "FireRing.h"
#include "Fireball.h"
#include "Upgrade.h"
#include "WizardBase.h"
#include "WizardData.h"
#include "WizardIds.h"
#include "WizardTypes.h"

class Crystal : public WizardBase {
   public:
    Crystal();

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onClick(Event::MouseButton b, bool clicked);
    void onHit(WizardId src, const Number& val);

    void calcMagicEffect();
    void drawMagic();

    std::unique_ptr<FireRing>& createFireRing();

    TargetObservable::SubscriptionPtr mTargetSub;
    UpgradeList::SubscriptionPtr mMagicEffectDisplay;

    std::vector<std::unique_ptr<FireRing>> mFireRings;

    TextRenderData mMagicText;
};

#endif
