#include "Fireball.h"

// FireballObservable
void FireballObservable::onSubscribe(SubscriptionPtr sub) {
    sub->get<FUNC>()(mTargets[sub->get<DATA>()]);
}

void FireballObservable::next(WizardId id, SDL_FPoint pos) {
    if (id == size) {
        throw std::runtime_error("Cannot use size as a target");
    }
    mTargets[id] = pos;
    for (auto sub : *this) {
        if (sub->get<DATA>() == id) {
            sub->get<FUNC>()(pos);
        }
    }
}

// TargetObservable
void TargetObservable::next(WizardId target, Number val, WizardId src) {
    for (auto sub : *this) {
        if (sub->get<DATA>() == target) {
            sub->get<FUNC>()(src, val);
        }
    }
}

// Fireball
const int Fireball::MAX_SPEED = 150;
const int Fireball::COLLIDE_ERR = 10;
const std::string Fireball::IMG = "res/projectiles/fireball.png";

Fireball::Fireball(float cX, float cY, WizardId src, WizardId target,
                   Number val)
    : mPos(std::make_shared<UIComponent>(Rect(), 0)),
      mTargetId(target),
      mSrcId(src),
      mVal(val) {
    mImg.texture = AssetManager::getTexture(IMG);
    mImg.dest = Rect(0, 0, 50, 50);
    mImg.fitToTexture();
    mImg.dest.setPos(cX, cY, Rect::Align::CENTER);
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
}

bool Fireball::dead() const { return mDead; }

void Fireball::onResize(ResizeData data) {
    mPos->rect.moveFactor((float)data.newW / data.oldW,
                          (float)data.newH / data.oldH);
}

void Fireball::onUpdate(Time dt) {
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
    } else {
        mPos->rect.move(MAX_SPEED * dt.s(), dx, dy);
        mImg.dest = mPos->rect;
    }
}

void Fireball::onRender(SDL_Renderer* renderer) { TextureBuilder().draw(mImg); }
