#include "TimeWizClock.h"

// TimeWizClock
const float TimeWizClock::SMALL_HAND_SPD = 20,
            TimeWizClock::LARGE_HAND_SPD = 1200;
const std::string TimeWizClock::CLOCK = "res/wizards/clock.png";
const std::string TimeWizClock::SMALL_HAND = "res/wizards/clock_hand_small.png";
const std::string TimeWizClock::LARGE_HAND = "res/wizards/clock_hand_large.png";

TimeWizClock::TimeWizClock(Rect r)
    : mPos(std::make_shared<UIComponent>(r, Elevation::WIZARDS)) {
    mClock.set(CLOCK);
    mSmallHand.set(SMALL_HAND).setRotationDeg(mSmallRot);
    mLargeHand.set(LARGE_HAND).setRotationDeg(mLargeRot);
    setRect(r);
}

void TimeWizClock::setRect(const Rect& r) {
    mPos->rect.setDim(r.w() * 2 / 3, r.h() * 2 / 3);
    mPos->rect.setPos(r.cX(), r.y(), Rect::Align::CENTER,
                      Rect::Align::BOT_RIGHT);
    mClock.setDest(mPos->rect);
    mSmallHand.setDest(mPos->rect);
    mLargeHand.setDest(mPos->rect);
}

void TimeWizClock::init() {
    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [this](SDL_Renderer* r) { onRender(r); }, mPos);
    mUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            [this](Time dt) { onUpdate(dt); });
}

void TimeWizClock::onRender(SDL_Renderer* r) {
    TextureBuilder tex;

    tex.draw(mClock);
    tex.draw(mSmallHand);
    tex.draw(mLargeHand);
}

void TimeWizClock::onUpdate(Time dt) {
    float multi =
        ParameterSystem::Param<TIME_WIZARD>(TimeWizardParams::ClockSpeed)
            .get()
            .toFloat();
    mSmallRot = fmodf(mSmallRot + SMALL_HAND_SPD * dt.s() * multi, 360);
    mSmallHand.setRotationDeg(mSmallRot);
    mLargeRot = fmodf(mLargeRot + LARGE_HAND_SPD * dt.s() * multi, 360);
    mLargeHand.setRotationDeg(mLargeRot);
}
