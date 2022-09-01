#include "Fireball.h"

// Fireball
const int Fireball::COLLIDE_ERR = 10;
const int Fireball::MAX_SPEED = 150;

const Rect Fireball::IMG_RECT(0, 0, 40, 40);

constexpr int FIREBALL_BASE_ROT_DEG = -45;

Fireball::Fireball(SDL_FPoint c, WizardId target, const std::string& img,
                   float maxSpeedMult)
    : Fireball(c, target, AnimationData{img}, maxSpeedMult) {}
Fireball::Fireball(SDL_FPoint c, WizardId target, const AnimationData& img,
                   float maxSpeedMult)
    : mPos(std::make_shared<UIComponent>(Rect(), Elevation::PROJECTILES)),
      mTargetId(target),
      mImgAnim(img),
      mMaxSpeed(MAX_SPEED * maxSpeedMult) {
    Rect imgR = IMG_RECT;
    imgR.setPos(c.x, c.y, Rect::Align::CENTER);
    mImg.set(img).setDest(imgR);
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
    if (mImgAnim.num_frames > 1) {
        mAnimTimerSub =
            ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
                [this](Timer& t) {
                    mImg.nextFrame();
                    return true;
                },
                mImgAnim);
    }

    launch(mTargetPos);
}

bool Fireball::dead() const { return mDead; }

void Fireball::launch(SDL_FPoint target) {
    // Start at half max speed
    float dx = target.x - mPos->rect.cX(), dy = target.y - mPos->rect.cY();
    float frac = mMaxSpeed / 3 / sqrtf(dx * dx + dy * dy);
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

float Fireball::getSpeed() const { return mMaxSpeed; }
void Fireball::setSpeed(float speed) { mMaxSpeed = speed; }

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
    float d = sqrtf(dx * dx + dy * dy);
    // Check in case we were moved onto the target
    if (d < COLLIDE_ERR) {
        onDeath();
        mDead = true;
        mResizeSub.reset();
        mRenderSub.reset();
        mUpdateSub.reset();
        mTargetSub.reset();
    } else {
        d = fminf(d / 2, mMaxSpeed);
        float v_squared =
            fmaxf(mV.x * mV.x + mV.y * mV.y, mMaxSpeed * mMaxSpeed / 4);
        float frac = v_squared / (d * d);
        mA.x = dx * frac;
        mA.y = dy * frac;

        float moveX = mV.x * sec + mA.x * aCoeff,
              moveY = mV.y * sec + mA.y * aCoeff;
        mV.x += mA.x * sec;
        mV.y += mA.y * sec;

        // Cap speed
        float mag = sqrtf(mV.x * mV.x + mV.y * mV.y);
        if (mag > mMaxSpeed) {
            frac = mMaxSpeed / mag;
            mV.x *= frac;
            mV.y *= frac;
        }

        float theta = 0;
        if (moveX != 0) {
            theta = atanf(moveY / moveX) / DEG_TO_RAD;
            if (moveX < 0) {
                theta += 180;
            }
        }
        theta += FIREBALL_BASE_ROT_DEG;

        Rect imgR = mImg.getRect();
        imgR.move(moveX, moveY);
        mImg.setDest(imgR).setRotationDeg(theta);
        mPos->rect = mImg.getDest();
    }
}

void Fireball::onRender(SDL_Renderer* renderer) { TextureBuilder().draw(mImg); }

void Fireball::onDeath() {}
