#ifndef CATALYST_H
#define CATALYST_H

#include <Components/CatalystRing.h>
#include <Components/Fireballs/WizardFireball.h>
#include <Components/Upgrade.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <Systems/IconSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Wizards/Definitions/CatalystDefs.h>
#include <Wizards/Message.h>
#include <Wizards/Money.h>
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
    void onWizFireballHit(const WizardFireball& fireball);

    Number calcMagicEffect();
    Number calcRange();
    Number calcGain(Number magic);
    void drawMagic();
    void updateRange();

    void setPos(float x, float y);

    TimeSystem::TimerObservable::SubscriptionPtr mAnimTimerSub;
    WizardFireball::HitObservable::IdSubscriptionPtr mWizFireballSub;
    UpgradeList::SubscriptionPtr mMagicEffectDisplay, mRangeUp, mZapCntUp,
        mShardUp;

    MessageHandlerPtr mMessages;

    CircleShape mRange;
    TextDataPtr mMagicText = std::make_shared<TextData>();
    RenderData mMagicRender;
};

#endif
