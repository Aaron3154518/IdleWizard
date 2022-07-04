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

class Wizard : public Component {
   public:
    Wizard() = default;
    ~Wizard() = default;

   private:
    void init();

    void setImage(const std::string& img);

    RectData mBorder;
    RenderData mImg;

    DragComponentPtr mPos = std::make_shared<DragComponent>(Rect(), 1, 500);
    RenderObservable::SubscriptionPtr mRenderSub;
    MouseObservable::SubscriptionPtr mMouseSub;
    DragObservable::SubscriptionPtr mDragSub;

    const static std::string IMGS[];
    int imgIdx = 0;
};

#endif