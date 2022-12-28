#include "TimeWizClock.h"

namespace TimeWizard {
// Clock
const float Clock::SMALL_HAND_SPD = 20, Clock::LARGE_HAND_SPD = 1200;
const std::string Clock::CLOCK = "res/wizards/clock.png";
const std::string Clock::SMALL_HAND = "res/wizards/clock_hand_small.png";
const std::string Clock::LARGE_HAND = "res/wizards/clock_hand_large.png";

Clock::Clock(Rect r)
    : mPos(std::make_shared<UIComponent>(r, Elevation::WIZARDS)) {
    mClock.set(CLOCK);
    mSmallHand.set(SMALL_HAND);
    mSmallHand.setRotationDeg(mSmallRot);
    mLargeHand.set(LARGE_HAND);
    mLargeHand.setRotationDeg(mLargeRot);
    setRect(r);
}

void Clock::setRect(const Rect& r) {
    mPos->rect.setDim(r.w() * 2 / 3, r.h() * 2 / 3);
    mPos->rect.setPos(r.cX(), r.y(), Rect::Align::CENTER,
                      Rect::Align::BOT_RIGHT);
    mClock.setDest(mPos->rect);
    mSmallHand.setDest(mPos->rect);
    mLargeHand.setDest(mPos->rect);
}

void Clock::init() {
    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [this](SDL_Renderer* r) { onRender(r); }, mPos);
    mUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            [this](Time dt) { onUpdate(dt); });
}

void Clock::onRender(SDL_Renderer* r) {
    TextureBuilder tex;

    tex.draw(mClock);
    tex.draw(mSmallHand);
    tex.draw(mLargeHand);
}

void Clock::onUpdate(Time dt) {
    float multi =
        ParameterSystem::Param<TIME_WIZARD>(TimeWizardParams::ClockSpeed)
            .get()
            .toFloat();
    mSmallRot = fmodf(mSmallRot + SMALL_HAND_SPD * dt.s() * multi, 360);
    mSmallHand.setRotationDeg(mSmallRot);
    mLargeRot = fmodf(mLargeRot + LARGE_HAND_SPD * dt.s() * multi, 360);
    mLargeHand.setRotationDeg(mLargeRot);
}
}  // namespace TimeWizard
