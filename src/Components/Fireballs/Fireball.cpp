#include "Fireball.h"

// Fireball
const int Fireball::COLLIDE_ERR = 10;
const int Fireball::MAX_SPEED = 150;
const int Fireball::ACCELERATION = 300;
const int Fireball::ACCEL_ZONE = 100;

// const int Fireball::DEF_VALUE_KEY = 0;

const Rect Fireball::IMG_RECT(0, 0, 40, 40);

Fireball::Fireball(SDL_FPoint c, WizardId target, const std::string& img)
    : mPos(std::make_shared<UIComponent>(Rect(), Elevation::PROJECTILES)),
      mTargetId(target) {
    mImg.texture = AssetManager::getTexture(img);
    mImg.dest.setPos(c.x, c.y, Rect::Align::CENTER);
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
    mTargetSub =
        ServiceSystem::Get<FireballService, FireballTargetPosObservable>()
            ->subscribe([this](SDL_FPoint p) { mTargetPos = p; }, mTargetId);

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
    mImg.dest.setDim(IMG_RECT.w() * size, IMG_RECT.h() * size,
                     Rect::Align::CENTER);
    mImg.shrinkToTexture();
    mPos->rect = mImg.dest;
}

void Fireball::setPos(float x, float y) {
    mPos->rect.setPos(x, y, Rect::Align::CENTER);
    mImg.dest = mPos->rect;
}

WizardId Fireball::getTargetId() const { return mTargetId; }

void Fireball::onResize(ResizeData data) {
    mPos->rect.moveFactor((float)data.newW / data.oldW,
                          (float)data.newH / data.oldH);
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
        mPos->rect.move(mV.x * sec + mA.y * aCoeff, mV.y * sec + mA.y * aCoeff);
        mV.x += mA.x * sec;
        mV.y += mA.y * sec;
        // Cap speed
        mag = std::sqrt(mV.x * mV.x + mV.y * mV.y);
        if (mag > MAX_SPEED) {
            frac = MAX_SPEED / mag;
            mV.x *= frac;
            mV.y *= frac;
        }
        mImg.dest = mPos->rect;
    }
}

void Fireball::onRender(SDL_Renderer* renderer) { TextureBuilder().draw(mImg); }

void Fireball::onDeath() {}
