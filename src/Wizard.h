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

class Wizard : public WizardBase {
   public:
    Wizard();

    const std::shared_ptr<WizardData>& getData() const;

    const static std::string POWER_UP_IMG, SPEED_UP_IMG;

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onClick(Event::MouseButton b, bool clicked);
    bool onTimer();
    void onWizardUpdate(const WizardsData& params);

    void shootFireball();

    const std::shared_ptr<WizardData> mData = std::make_shared<WizardData>();

    TimerObservable::SubscriptionPtr mTimerSub;
    WizardsDataObservable::SubscriptionPtr mWizUpdateSub;
    UpgradeList::SubscriptionPtr mTargetUp, mPowerUp, mSpeedUp;

    WizardId mTarget = WizardId::CRYSTAL;

    Number mBasePower = 1, mPower = 1;
    std::vector<std::unique_ptr<Fireball>> mFireballs;
};

class Crystal : public WizardBase {
   public:
    Crystal();

    const std::shared_ptr<CrystalData>& getData() const;

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onHit(WizardId src, Number val);

    const std::shared_ptr<CrystalData> mData = std::make_shared<CrystalData>();

    TargetObservable::SubscriptionPtr mTargetSub;

    TextRenderData mMagicText;
};

class Catalyst : public WizardBase {
   public:
    Catalyst();

    const std::shared_ptr<CatalystData>& getData() const;

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onHit(WizardId src, Number val);

    const std::shared_ptr<CatalystData> mData =
        std::make_shared<CatalystData>();

    TargetObservable::SubscriptionPtr mTargetSub;

    TextRenderData mMagicText;
};

#endif