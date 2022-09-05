#include "Message.h"

MessageHandler::MessageHandler(const FontData& font) {
    mMsgTData.setFont(font);
}

void MessageHandler::init() {
    mUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            [this](Time dt) { onUpdate(dt); });
}

void MessageHandler::onUpdate(Time dt) {
    for (auto it = mMessages.begin(), end = mMessages.end(); it != end; ++it) {
        it->mTimer -= dt.ms();
        if (it->mTimer <= 0) {
            if (it->mMoving) {
                it->mTimer = rDist(gen) * 1000 + 750;
                it->mMoving = false;
            } else {
                it = mMessages.erase(it);
                if (it == end) {
                    break;
                }
            }
        } else if (it->mMoving) {
            Rect msgR = it->mRData.getRect();
            msgR.move(it->mTrajectory.x * dt.s(), it->mTrajectory.y * dt.s());
            it->mRData.setDest(msgR);
        }
    }
}

void MessageHandler::draw(TextureBuilder tex) {
    for (auto msg : mMessages) {
        tex.draw(msg.mRData);
    }
}

void MessageHandler::addMessage(const Rect& r, const std::string& msg,
                                SDL_Color color) {
    mMsgTData.setText(msg).setColor(color);

    float dx = (rDist(gen) - .5) * r.halfW(),
          dy = (rDist(gen) - .5) * r.halfH();
    RenderData rData = RenderData()
                           .set(mMsgTData)
                           .setFit(RenderData::FitMode::Texture)
                           .setDest(Rect(r.cX() + dx, r.cY() + dy, 0, 0));

    SDL_FPoint trajectory{copysignf(rDist(gen), dx), copysignf(rDist(gen), dy)};
    if (trajectory.x == 0 && trajectory.y == 0) {
        trajectory.y = 1;
    }
    float mag = sqrt(trajectory.x * trajectory.x + trajectory.y * trajectory.y);
    float maxSpeed = sqrt(r.w() * r.w() + r.h() * r.h());
    trajectory.x *= maxSpeed / mag;
    trajectory.y *= maxSpeed / mag;

    mMessages.push_back(
        Message{rData, (int)(rDist(gen) * 250) + 250, true, trajectory});
}
