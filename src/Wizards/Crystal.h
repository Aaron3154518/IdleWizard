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

    void onRender(SDL_Renderer* r);
    void onUpdate(Time dt);
    void onClick(Event::MouseButton b, bool clicked);
    void onHide(WizardId id, bool hide);
    void onWizEvent(WizardSystem::Event e);
    void onFireballHit(const Fireball& fireball);

    void calcMagicEffect();
    void drawMagic();

    std::unique_ptr<FireRing>& createFireRing(const Number& val);

    void addMessage(const std::string& msg);

    UpdateObservable::SubscriptionPtr mUpdateSub;
    Fireball::HitObservable::IdSubscriptionPtr mFireballSub;
    UpgradeList::SubscriptionPtr mMagicEffectDisplay, mPowWizBuy, mTimeWizBuy;

    TextData mMsgTData;
    std::vector<Message> mMessages;

    std::vector<std::unique_ptr<FireRing>> mFireRings;

    TextRenderData mMagicText;
};

#endif
