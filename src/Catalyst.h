#ifndef CATALYST_H
#define CATALYST_H

#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>

#include <memory>

#include "Fireball.h"
#include "Upgrade.h"
#include "WizardBase.h"
#include "WizardData.h"
#include "WizardIds.h"
#include "WizardTypes.h"

class Catalyst : public WizardBase {
   public:
    Catalyst();

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onHit(WizardId src, Number val);

    void calcMagicEffect();
    void drawMagic();

    TargetObservable::SubscriptionPtr mTargetSub;
    UpgradeList::SubscriptionPtr mMagicEffectDisplay;

    TextRenderData mMagicText;
};

#endif
