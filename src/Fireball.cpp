#include "Fireball.h"

// FireballObservable
void FireballObservable::onSubscribe(SubscriptionPtr sub) {
    sub->get<FUNC>()(mTargets[sub->get<DATA>()]);
}

void FireballObservable::next(WizardId id, SDL_FPoint pos) {
    if (id == WizardId::size) {
        throw std::runtime_error("Cannot use size as a target");
    }
    mTargets[id] = pos;
    for (auto sub : *this) {
        if (sub->get<DATA>() == id) {
            sub->get<FUNC>()(pos);
        }
    }
}

SDL_FPoint FireballObservable::getPos(WizardId id) const {
    if (id == WizardId::size) {
        throw std::runtime_error("Cannot use size as a target");
    }
    return mTargets[id];
}

// TargetObservable
void TargetObservable::next(WizardId target, const Number& val, WizardId src) {
    for (auto sub : *this) {
        if (sub->get<DATA>() == target) {
            sub->get<FUNC>()(src, val);
        }
    }
}

// Fireball
const int Fireball::COLLIDE_ERR = 10;
const int Fireball::MAX_SPEED = 150;
const int Fireball::ACCELERATION = 300;
const int Fireball::ACCEL_ZONE = 100;

Fireball::Fireball(SDL_FPoint c, WizardId src, WizardId target, Number val,
                   const std::string& img)
    : mPos(std::make_shared<UIComponent>(Rect(), 0)),
      mTargetId(target),
      mSrcId(src),
      mVal(val) {
    mImg.texture = AssetManager::getTexture(img);
    mImg.dest = Rect(0, 0, 50, 50);
    mImg.fitToTexture();
    mImg.dest.setPos(c.x, c.y, Rect::Align::CENTER);
    mPos->rect = mImg.dest;
}

void Fireball::init() {
    mResizeSub =
        ServiceSystem::Get<ResizeService, ResizeObservable>()->subscribe(
            std::bind(&Fireball::onResize, this, std::placeholders::_1));
    mUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            std::bind(&Fireball::onUpdate, this, std::placeholders::_1));
    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&Fireball::onRender, this, std::placeholders::_1), mPos);
    mFireballSub =
        ServiceSystem::Get<FireballService, FireballObservable>()->subscribe(
            [this](SDL_FPoint p) { mTargetPos = p; }, mTargetId);
    mFireRingSub =
        ServiceSystem::Get<FireRingService, FireRingObservable>()->subscribe(
            std::bind(&Fireball::onFireRing, this, std::placeholders::_1),
            mPos);

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
        mDead = true;
        ServiceSystem::Get<FireballService, TargetObservable>()->next(
            mTargetId, mVal, mSrcId);
        mResizeSub.reset();
        mRenderSub.reset();
        mUpdateSub.reset();
        mFireballSub.reset();
        mFireRingSub.reset();
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

void Fireball::onFireRing(const Number& effect) {
    if (mSrcId == WIZARD && !mHitFireRing) {
        mHitFireRing = true;
        mVal ^= effect;
    }
}
