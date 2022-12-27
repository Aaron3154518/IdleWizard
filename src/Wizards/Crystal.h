#ifndef CRYSTAL_H
#define CRYSTAL_H

#include <Components/FireRing.h>
#include <Components/Fireballs/PoisonFireball.h>
#include <Components/Fireballs/PowerFireball.h>
#include <Components/Fireballs/WizardFireball.h>
#include <Components/FractureButton.h>
#include <Components/Upgrade.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/MouseService.h>
#include <Systems/IconSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Utils/AnimationData.h>
#include <Wizards/Definitions/CrystalDefs.h>
#include <Wizards/Definitions/PowerWizardDefs.h>
#include <Wizards/Definitions/WizardDefs.h>
#include <Wizards/Message.h>
#include <Wizards/Money.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>

#include <memory>
#include <vector>

class Crystal : public WizardBase {
   public:
    enum MagicSource : uint8_t {
        Fireball = 0,
        Glow,
        Poison,
    };

    Crystal();

   private:
    void init();
    void setSubscriptions();
    void setUpgrades();
    void setParamTriggers();

    void onRender(SDL_Renderer* r);
    void onClick(Event::MouseButton b, bool clicked);
    void onHide(bool hide);
    void onT1Reset();
    void onWizFireballHit(const WizardFireball& fireball);
    void onPowFireballHit(const PowerFireball& fireball);
    bool powFireballFilter(const PowerFireball& fireball);
    bool onGlowTimer(Timer& t);
    bool onGlowFinishTimer(Timer& t, const Number& magic);
    bool onPoisonTimer(Timer& t);

    Number calcMagicEffect();
    Number calcShardGain();
    Number calcNumWizards();
    Number calcWizCntEffect();
    Number calcGlowEffect();
    void drawMagic();

    void addMagic(MagicSource source, const Number& amnt, SDL_Color msgColor);

    int getAnimationDelay();

    std::unique_ptr<FireRing>& createFireRing(const Number& val);

    void setPos(float x, float y);

    TimeSystem::TimerObservable::SubscriptionPtr mAnimTimerSub, mGlowTimerSub,
        mGlowFinishTimerSub, mGlowAnimTimerSub, mPoisonTimerSub;
    WizardFireballList::HitObservable::SubscriptionPtr mWizFireballHitSub;
    PowerFireballList::HitObservable::SubscriptionPtr mPowFireballHitSub;
    WizardSystem::WizardEventObservable::IdSubscriptionPtr mT1ResetSub;
    UpgradeList::SubscriptionPtr mMagicEffectDisplay, mWizCntUp, mGlowUp,
        mPowWizBuy, mTimeWizBuy, mCatalystBuy, mPoisWizBuy, mRobotBuy;

    MessageHandlerPtr mMessages;

    bool mGlowFinishing = false;
    Number mGlowMagic;
    std::vector<std::unique_ptr<FireRing>> mFireRings;
    std::unique_ptr<FractureButton> mFractureBtn;

    TextDataPtr mMagicText = std::make_shared<TextData>();
    RenderData mMagicRender;
    RenderAnimation mGlowBkgrnd, mGlowFinishBkgrnd;
};

#endif
