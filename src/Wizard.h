#ifndef WIZARD_H
#define WIZARD_H

#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/MouseService/DragService.h>
#include <ServiceSystem/MouseService/MouseService.h>
#include <ServiceSystem/ServiceSystem.h>

#include <memory>
#include <random>
#include <vector>

#include "Fireball.h"
#include "Upgrade.h"
#include "WizardIds.h"
#include "WizardUpdate.h"

class WizardBase : public Component {
   public:
    virtual ~WizardBase();

    const static Rect BORDER_RECT, IMG_RECT;
    const static std::string IMGS[];
    const static FontData FONT;

   protected:
    WizardBase(WizardId id);

    virtual void init();

    virtual void onRender(SDL_Renderer* r);
    virtual void onClick(Event::MouseButton b, bool clicked);

    void setPos(float x, float y);

    void setImage(const std::string& img);

    const WizardId mId;

    RectData mBorder;
    RenderData mImg;

    DragComponentPtr mComp;
    RenderObservable::SubscriptionPtr mRenderSub;
    MouseObservable::SubscriptionPtr mMouseSub;
    DragObservable::SubscriptionPtr mDragSub;

    std::mt19937 gen = std::mt19937(rand());
    std::uniform_int_distribution<> dist =
        std::uniform_int_distribution<>(1, WizardId::size - 1);
    std::uniform_real_distribution<> rDist;
};

class Wizard : public WizardBase {
   public:
    Wizard();

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onClick(Event::MouseButton b, bool clicked);
    void onWizardUpdate(const ParameterList<WizardParams>& params);

    void shootFireball(WizardId target);

    WizardParameters::SubscriptionPtr mWizUpdateSub;

    std::vector<std::shared_ptr<Upgrade>> mUpgrades;
    bool boughtPower = false;

    Number mBasePower = Number(1);
    Number mPower = Number(1);
    std::vector<std::unique_ptr<Fireball>> mFireballs;
};

class Crystal : public WizardBase {
   public:
    Crystal();

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onHit(WizardId src, Number val);

    TextRenderData mMagicText;

    TargetObservable::SubscriptionPtr mTargetSub;

    Number mMagic;
};

class Catalyst : public WizardBase {
   public:
    Catalyst();

   private:
    void init();
};

#endif