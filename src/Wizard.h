#ifndef WIZARD_H
#define WIZARD_H

#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/DragService.h>
#include <ServiceSystem/EventServices/MouseService.h>
#include <ServiceSystem/EventServices/ResizeService.h>
#include <ServiceSystem/ServiceSystem.h>
#include <ServiceSystem/UpdateServices/TimerService.h>

#include <memory>
#include <random>
#include <string>
#include <vector>

#include "Fireball.h"
#include "Upgrade.h"
#include "WizardBase.h"
#include "WizardData.h"
#include "WizardIds.h"
#include "WizardTypes.h"

class Wizard : public WizardBase {
   public:
    Wizard();

    const static std::string POWER_UP_IMG, SPEED_UP_IMG;

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onClick(Event::MouseButton b, bool clicked);
    bool onTimer();

    void calcPower();

    void shootFireball();

    TimerObservable::SubscriptionPtr mTimerSub;
    UpgradeList::SubscriptionPtr mTargetUp, mPowerUp, mSpeedUp;

    WizardId mTarget = WizardId::CRYSTAL;

    std::vector<std::unique_ptr<Fireball>> mFireballs;
};

class Crystal : public WizardBase {
   public:
    Crystal();

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onHit(WizardId src, Number val);

    void calcMagicEffect();

    TargetObservable::SubscriptionPtr mTargetSub;

    TextRenderData mMagicText;
};

class Catalyst : public WizardBase {
   public:
    Catalyst();

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onHit(WizardId src, Number val);

    void calcMagicEffect();

    TargetObservable::SubscriptionPtr mTargetSub;

    TextRenderData mMagicText;
};

#endif