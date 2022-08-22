#ifndef CATALYST_H
#define CATALYST_H

#include <Components/Fireball.h>
#include <Components/Upgrade.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Systems/WizardSystem.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>

#include <memory>

class Catalyst : public WizardBase {
   public:
    Catalyst();

   private:
    void init();
    void setSubscriptions();
    void setUpgrades();
    void setParamTriggers();

    void onRender(SDL_Renderer* r);
    void onFireballHit(const Fireball& fireball);

    Number calcMagicEffect();
    void drawMagic();

    Fireball::HitObservable::IdSubscriptionPtr mFireballSub;
    UpgradeList::SubscriptionPtr mMagicEffectDisplay;

    TextRenderData mMagicText;

    const static std::vector<bool> DEFAULT_PARAMS;
};

#endif
