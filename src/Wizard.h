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
#include "WizardIds.h"

class Wizard : public Component {
   public:
    Wizard(WizardId id);
    ~Wizard() = default;

   private:
    void init();

    void onRender(SDL_Renderer* r);
    void onClick(Event::MouseButton b, bool clicked);

    void setPos(float x, float y);

    void setImage(const std::string& img);

    void shootFireball(WizardId target);

    const WizardId mId;

    RectData mBorder;
    RenderData mImg;

    DragComponentPtr mComp;
    RenderObservable::SubscriptionPtr mRenderSub;
    MouseObservable::SubscriptionPtr mMouseSub;
    DragObservable::SubscriptionPtr mDragSub;

    std::vector<std::unique_ptr<Fireball>> mFireballs;

    std::random_device rd;
    std::mt19937 gen;
    std::uniform_int_distribution<> dist;

    const static Rect BORDER_RECT, IMG_RECT;
    const static std::string IMGS[];
    int imgIdx = 0;
};

#endif