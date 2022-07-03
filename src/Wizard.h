#ifndef WIZARD_H
#define WIZARD_H

#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/MouseService/MouseService.h>
#include <ServiceSystem/ServiceSystem.h>

#include <memory>

class Wizard : public Component {
   public:
    Wizard() = default;
    ~Wizard() = default;

   private:
    void init();

    RenderData mImg;
    UIComponentPtr mPos = std::make_shared<UIComponent>(Rect(), 1);
    RenderObservable::SubscriptionPtr mRenderSub;
    MouseObservable::SubscriptionPtr mMouseSub;
    const static std::string IMGS[];
    int imgIdx = 0;
};

#endif