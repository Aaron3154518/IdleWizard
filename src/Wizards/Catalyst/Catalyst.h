#ifndef CATALYST_H
#define CATALYST_H

#include <Components/Message.h>
#include <Components/Upgrade.h>
#include <Components/WizardBase.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <Systems/IconSystem/IconSystem.h>
#include <Systems/IconSystem/MoneyIcons.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem/MagicObservables.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Wizards/Catalyst/CatalystConstants.h>
#include <Wizards/Catalyst/CatalystParameters.h>
#include <Wizards/Catalyst/CatalystRing.h>
#include <Wizards/Crystal/CrystalParameters.h>
#include <Wizards/PoisonWizard/PoisonFireball.h>
#include <Wizards/PoisonWizard/PoisonWizardParameters.h>
#include <Wizards/Wizard/WizardFireball.h>
#include <Wizards/WizardIds.h>

#include <memory>

namespace Catalyst {
class Catalyst : public WizardBase {
   public:
    Catalyst();

   private:
    void init();
    void setSubscriptions();
    void setUpgrades();
    void setParamTriggers();

    void onRender(SDL_Renderer* r);
    void onWizFireballHit(const Wizard::Fireball& fireball);
    void onPoisFireballHit(const PoisonWizard::Fireball& fireball);
    void onMagic(const Number& amnt);

    Number calcMagicEffect();
    Number calcRange();
    Number calcFbCntEffect();
    void drawMagic();
    void updateRange();

    void setPos(float x, float y);

    TimeSystem::TimerObservable::SubscriptionPtr mAnimTimerSub;
    Wizard::FireballList::HitObservable::SubscriptionPtr mWizFireballSub;
    PoisonWizard::FireballList::HitObservable::SubscriptionPtr mPoisFireballSub;
    WizardSystem::CatalystMagicObservable::SubscriptionPtr mMagicSub;
    UpgradeList::SubscriptionPtr mMagicEffectDisplay, mRangeUp, mZapCntUp,
        mZapperCntUp, mShardUp, mGainUp1, mGainUp2, mFbCountUp, mMultUp;

    MessageHandlerPtr mMessages;

    CircleShape mRange;
    TextDataPtr mMagicText = std::make_shared<TextData>();
    RenderData mMagicRender;
};
}  // namespace Catalyst

#endif
