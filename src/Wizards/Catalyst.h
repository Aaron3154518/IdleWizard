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
    typedef Observable<void(const Number&), UIComponentPtr> HitObservableBase;
    class HitObservable : public HitObservableBase, public Component {
        friend class Catalyst;

       public:
        enum : uint8_t { FUNC = 0, DATA };

        void setPos(const CircleData& pos);

       private:
        void init();

        bool onTimer(Timer& timer);

        CircleData mPos;

        TimeSystem::TimerObservable::SubscriptionPtr mTimerSub;

        std::mt19937 gen = std::mt19937(rand());
        std::uniform_real_distribution<float> rDist;
    };

   public:
    Catalyst();

    static void setDefaults();

   private:
    void init();
    void setSubscriptions();
    void setUpgrades();
    void setParamTriggers();

    void onRender(SDL_Renderer* r);
    void onFireballHit(const Fireball& fireball);

    Number calcMagicEffect();
    Number calcRange();
    void drawMagic();
    void updateRange();

    void setPos(float x, float y);

    Fireball::HitObservable::IdSubscriptionPtr mFireballSub;
    UpgradeList::SubscriptionPtr mMagicEffectDisplay, mRangeUp;

    CircleShape mRange;
    TextRenderData mMagicText;
};

class CatalystService : public Service<Catalyst::HitObservable> {};

#endif
