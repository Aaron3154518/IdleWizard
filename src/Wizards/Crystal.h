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
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>

#include <memory>
#include <vector>

class Crystal : public WizardBase {
   public:
    Crystal();

    const static Number T1_COST1, T1_COST2;
    const static SDL_Color MSG_COLOR;

    const static AnimationData IMG;

    struct Message {
        RenderData mRData;
        int mTimer;
        bool mMoving;
        SDL_FPoint mTrajectory;
    };

    static void setDefaults();

   private:
    void init();
    void setSubscriptions();
    void setUpgrades();
    void setParamTriggers();

    void onRender(SDL_Renderer* r);
    void onUpdate(Time dt);
    void onClick(Event::MouseButton b, bool clicked);
    void onHide(WizardId id, bool hide);
    void onReset(WizardSystem::ResetTier tier);
    void onWizFireballHit(const WizardFireball& fireball);
    void onPowFireballHit(const PowerWizFireball& fireball);

    Number calcMagicEffect();
    Number calcShardGain();
    void drawMagic();

    int getAnimationDelay();

    std::unique_ptr<FireRing>& createFireRing(const Number& val);

    void addMessage(const std::string& msg);

    void triggerT1Reset();

    void setPos(float x, float y);

    UpdateObservable::SubscriptionPtr mUpdateSub;
    TimeSystem::TimerObservable::SubscriptionPtr mAnimTimerSub;
    WizardFireball::HitObservable::IdSubscriptionPtr mWizFireballHitSub;
    PowerWizFireball::HitObservable::IdSubscriptionPtr mPowFireballHitSub;
    UpgradeList::SubscriptionPtr mMagicEffectDisplay, mPowWizBuy, mTimeWizBuy,
        mCatalystBuy;

    TextData mMsgTData;
    std::vector<Message> mMessages;

    std::vector<std::unique_ptr<FireRing>> mFireRings;

    TextData mMagicText;
    RenderData mMagicRender;
};

#endif
