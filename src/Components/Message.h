#ifndef MESSAGE_H
#define MESSAGE_H

#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/TextRender.h>
#include <RenderSystem/TextureBuilder.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/UpdateService.h>

#include <memory>
#include <random>
#include <vector>

struct Message {
    RenderData mRData;
    int mTimer;
    bool mMoving;
    SDL_FPoint mTrajectory;
};

class MessageHandler : public Component {
   public:
    MessageHandler(const FontData& font);

    void addMessage(const Rect& r, const std::string& msg, SDL_Color color);

    void draw(TextureBuilder tex);

   private:
    void init();

    void onUpdate(Time dt);

    std::mt19937 gen = std::mt19937(rand());
    std::uniform_real_distribution<float> rDist;

    TextData mMsgTData;
    std::vector<Message> mMessages;

    UpdateObservable::SubscriptionPtr mUpdateSub;
};

typedef std::unique_ptr<MessageHandler> MessageHandlerPtr;

#endif