#ifndef MESSAGE_H
#define MESSAGE_H

#include <RenderSystem/RenderTypes.h>

struct Message {
    RenderData mRData;
    int mTimer;
    bool mMoving;
    SDL_FPoint mTrajectory;
};

#endif