#include "Bot.h"

// UpgradEBot
UpgradeBot::UpgradeBot()
    : mPos(std::make_shared<UIComponent>(Rect(0, 0, 50, 50), Elevation::BOTS)) {
}

void UpgradeBot::init() {
    mImg.set(RobotWizardDefs::IMG());

    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [this](SDL_Renderer* r) { onRender(r); }, mPos);

    mUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            [this](Time dt) { onUpdate(dt); });

    mAnimTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            mImg->nextFrame();
            return true;
        },
        RobotWizardDefs::IMG().frame_ms);
}

void UpgradeBot::onRender(SDL_Renderer* r) {
    TextureBuilder tex;

    mImg.setDest(mPos->rect);
    tex.draw(mImg);
}
void UpgradeBot::onUpdate(Time dt) {
    Rect robPos = WizardSystem::GetWizardPosObservable()->get(ROBOT_WIZARD);

    float x = mPos->rect.cX(), y = mPos->rect.cY();
    float rX = robPos.cX(), rY = robPos.cY();
    float dx = rX - x, dy = rY - y;
    float mag = sqrtf(dx * dx + dy * dy);

    // Oscillate speed and radius
    float rad = 100 + 40 * (sinf(mTheta * 7 / 11) + .25);
    float maxSpd = 75 * (sinf(mTheta * 7 / 19) + 1.5);

    float x_t = rad * cosf(mTheta) + rX, y_t = rad / 3 * sinf(mTheta) + rY;
    float dx_t = x_t - x, dy_t = y_t - y;
    float mag_t = sqrtf(dx_t * dx_t + dy_t * dy_t);

    float s = dt.s();
    // Smooth speed transition
    mV.x = mV.x * (1 - s) + (dx_t * maxSpd / mag_t) * s;
    mV.y = mV.y * (1 - s) + (dy_t * maxSpd / mag_t) * s;
    if (mag <= rad + 10) {
        mTheta += s * M_PI / 3;
    }

    mPos->rect.move(mV.x * s, mV.y * s);
}
