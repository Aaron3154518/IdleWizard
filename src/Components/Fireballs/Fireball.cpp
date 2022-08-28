#include "Fireball.h"

// Fireball
const int Fireball::COLLIDE_ERR = 10;
const int Fireball::MAX_SPEED = 150;
const int Fireball::ACCELERATION = 300;
const int Fireball::ACCEL_ZONE = 100;

const unsigned int Fireball::MSPF = 100, Fireball::NUM_FRAMES = 6;
const std::string Fireball::IMG = "res/projectiles/fireball_ss.png";

// const int Fireball::DEF_VALUE_KEY = 0;

const Rect Fireball::IMG_RECT(0, 0, 40, 40);

Fireball::Fireball(SDL_FPoint c, WizardId target, const std::string& img)
    : mPos(std::make_shared<UIComponent>(Rect(), Elevation::PROJECTILES)),
      mTargetId(target) {
    Rect imgR = IMG_RECT;
    imgR.setPos(c.x, c.y, Rect::Align::CENTER);
    mImg.set(IMG, NUM_FRAMES).setDest(imgR);
    setSize(1);
}

void Fireball::init() {
    mResizeSub =
        ServiceSystem::Get<ResizeService, ResizeObservable>()->subscribe(
            [this](ResizeData d) { onResize(d); });
    mUpdateSub = TimeSystem::GetUpdateObservable()->subscribe(
        [this](Time dt) { onUpdate(dt); });
    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [this](SDL_Renderer* r) { onRender(r); }, mPos);
    mTargetSub = WizardSystem::GetWizardPosObservable()->subscribe(
        [this](SDL_FPoint p) { mTargetPos = p; }, mTargetId);
    mAnimTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            [this](Timer& t) {
                mImg.nextFrame();
                return true;
            },
            Timer(MSPF));

    launch(mTargetPos);
}

bool Fireball::dead() const { return mDead; }

void Fireball::launch(SDL_FPoint target) {
    // Start at half max speed
    float dx = target.x - mPos->rect.cX(), dy = target.y - mPos->rect.cY();
    float frac = MAX_SPEED / std::sqrt(dx * dx + dy * dy) / 2;
    mV.x = dx * frac;
    mV.y = dy * frac;
}

float Fireball::getSize() const { return mSize; }
void Fireball::setSize(float size) {
    mSize = fmin(fmax(size, 0), 50);
    Rect imgR = mImg.getRect();
    imgR.setDim(IMG_RECT.w() * size, IMG_RECT.h() * size, Rect::Align::CENTER);
    mImg.setDest(imgR);
    mPos->rect = mImg.getDest();
}

void Fireball::setPos(float x, float y) {
    Rect imgR = mImg.getRect();
    imgR.setPos(x, y, Rect::Align::CENTER);
    mImg.setDest(imgR);
    mPos->rect = mImg.getDest();
}

WizardId Fireball::getTargetId() const { return mTargetId; }

void Fireball::onResize(ResizeData data) {
    Rect imgR = mImg.getRect();
    imgR.moveFactor((float)data.newW / data.oldW, (float)data.newH / data.oldH);
    mImg.setDest(imgR);
    mPos->rect = mImg.getDest();
}

void Fireball::onUpdate(Time dt) {
    float sec = dt.s();
    float aCoeff = sec * sec / 2;
    float dx = mTargetPos.x - mPos->rect.cX(),
          dy = mTargetPos.y - mPos->rect.cY();
    float mag = std::sqrt(dx * dx + dy * dy);
    // Check in case we were moved onto the target
    if (mag < COLLIDE_ERR) {
        onDeath();
        mDead = true;
        mResizeSub.reset();
        mRenderSub.reset();
        mUpdateSub.reset();
        mTargetSub.reset();
    } else {
        float frac = fmax((ACCEL_ZONE / mag), 1) * ACCELERATION / mag;
        mA.x = dx * frac;
        mA.y = dy * frac;
        float dx = mV.x * sec + mA.x * aCoeff, dy = mV.y * sec + mA.y * aCoeff;
        mV.x += mA.x * sec;
        mV.y += mA.y * sec;
        // Cap speed
        mag = std::sqrt(mV.x * mV.x + mV.y * mV.y);
        if (mag > MAX_SPEED) {
            frac = MAX_SPEED / mag;
            mV.x *= frac;
            mV.y *= frac;
        }

        float theta = 0;
        if (dx != 0) {
            theta = atanf(dy / dx) / DEG_TO_RAD;
            if (dx < 0) {
                theta += 180;
            }
        }
        theta -= 45;

        Rect imgR = mImg.getRect();
        imgR.move(dx, dy);
        mImg.setDest(imgR).setRotationDeg(theta);
        mPos->rect = mImg.getDest();
    }
}

void Fireball::onRender(SDL_Renderer* renderer) { TextureBuilder().draw(mImg); }

void Fireball::onDeath() {}
