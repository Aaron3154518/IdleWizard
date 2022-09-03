#ifndef CRYSTAL_H
#define CRYSTAL_H

#include <Components/FireRing.h>
#include <Components/Fireballs/PowerWizFireball.h>
#include <Components/Fireballs/WizardFireball.h>
#include <Components/Upgrade.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/MouseService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Utils/AnimationData.h>
#include <Wizards/Definitions/CrystalDefs.h>
#include <Wizards/Message.h>
#include <Wizards/Money.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>

#include <memory>
#include <vector>

class Crystal : public WizardBase {
   public:
    Crystal();

   private:
    void init();
    void setSubscriptions();
    void setUpgrades();
    void setParamTriggers();

    void onRender(SDL_Renderer* r);
    void onUpdate(Time dt);
    void onClick(Event::MouseButton b, bool clicked);
    void onHide(bool hide);
    void onT1Reset();
    void onWizFireballHit(const WizardFireball& fireball);
    void onPowFireballHit(const PowerWizFireball& fireball);
    bool onGlowTimer(Timer& t);

    Number calcMagicEffect();
    Number calcShardGain();
    Number calcNumWizards();
    Number calcWizCntEffect();
    Number calcGlowEffect();
    void drawMagic();

    int getAnimationDelay();

    std::unique_ptr<FireRing>& createFireRing(const Number& val);

    void addMessage(const std::string& msg);

    void triggerT1Reset();

    void setPos(float x, float y);

    UpdateObservable::SubscriptionPtr mUpdateSub;
    TimeSystem::TimerObservable::SubscriptionPtr mAnimTimerSub, mGlowTimerSub,
        mGlowAnimTimerSub;
    WizardFireball::HitObservable::IdSubscriptionPtr mWizFireballHitSub;
    PowerWizFireball::HitObservable::IdSubscriptionPtr mPowFireballHitSub;
    WizardSystem::WizardEventObservable::IdSubscriptionPtr mT1ResetSub;
    UpgradeList::SubscriptionPtr mMagicEffectDisplay, mWizCntUp, mGlowUp,
        mPowWizBuy, mTimeWizBuy, mCatalystBuy;

    TextData mMsgTData;
    std::vector<Message> mMessages;

    Number mGlowMagic;
    std::vector<std::unique_ptr<FireRing>> mFireRings;

    TextDataPtr mMagicText = std::make_shared<TextData>();
    RenderData mMagicRender, mGlowBkgrnd;
};

#endif
