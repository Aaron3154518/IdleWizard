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
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Wizards/WizardBase.h>
#include <Wizards/WizardIds.h>

#include <memory>
#include <vector>

class Crystal : public WizardBase {
   public:
    Crystal();

    const static Number T1_COST1, T1_COST2;
    const static SDL_Color MSG_COLOR;

    struct Message {
        RenderData mRData;
        int mTimer;
        bool mMoving;
        SDL_FPoint mTrajectory;
    };

   private:
    void init();
    void setSubscriptions();
    void setUpgrades();
    void setParamTriggers();

    void onRender(SDL_Renderer* r);
    void onUpdate(Time dt);
    void onClick(Event::MouseButton b, bool clicked);
    void onHide(WizardId id, bool hide);
    void onResetT1();
    void onFireballHit(const Fireball& fireball);

    Number calcMagicEffect();
    Number calcShardGain();
    void drawMagic();

    std::unique_ptr<FireRing>& createFireRing(const Number& val);

    void addMessage(const std::string& msg);

    void triggerT1Reset();

    UpdateObservable::SubscriptionPtr mUpdateSub;
    Fireball::HitObservable::IdSubscriptionPtr mFireballSub;
    UpgradeList::SubscriptionPtr mMagicEffectDisplay, mPowWizBuy, mTimeWizBuy,
        mCatalystBuy;

    TextData mMsgTData;
    std::vector<Message> mMessages;

    std::vector<std::unique_ptr<FireRing>> mFireRings;

    TextRenderData mMagicText;

    const static std::vector<bool> DEFAULT_PARAMS;
};

#endif
