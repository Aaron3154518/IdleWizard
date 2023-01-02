#include "PoisonFireball.h"

namespace PoisonWizard {
// Fireball
Fireball::Fireball(SDL_FPoint c, WizardId target, const Data& data)
    : FireballBase(c, target),
      mSizeSum(data.sizeFactor),
      mPower(data.power),
      mDuration(data.duration) {
    setSize(data.sizeFactor);
    setSpeedFactor(data.speed);
    setImg(IconSystem::Get(PoisonWizard::Constants::GLOB_IMG()));
    mBubbleAImg.set(IconSystem::Get(PoisonWizard::Constants::BUBBLE1_IMG()));
    mBubbleBImg.set(IconSystem::Get(PoisonWizard::Constants::BUBBLE2_IMG()));
}

void Fireball::init() {
    FireballBase::init();

    mGlobHitSub.reset();
}

void Fireball::onUpdate(Time dt) {
    for (auto it = mBubbles.begin(); it != mBubbles.end();) {
        it->timer -= dt.ms();
        if (it->timer <= 0) {
            it = mBubbles.erase(it);
            continue;
        }

        it->pos.move(it->v.x * dt.s(), it->v.y * dt.s());
        ++it;
    }

    if (rDist(gen) < dt.s() * 5) {
        float theta = rDist(gen) * 2 * M_PI, speed = rDist(gen) * 50 + 25,
              w = mPos->rect.minDim() * (rDist(gen) + .5) * .5f;
        Rect r(0, 0, w, w);
        r.setPos(mPos->rect, Rect::Align::CENTER);
        mBubbles.push_back({rDist(gen) < .5f ? BubbleType::A : BubbleType::B,
                            r,
                            {speed * cosf(theta), speed * sinf(theta)},
                            (int)(rDist(gen) * 750 + 250)});
    }

    FireballBase::onUpdate(dt);
}
void Fireball::draw(TextureBuilder& tex) {
    for (auto& bubble : mBubbles) {
        switch (bubble.type) {
            case BubbleType::A:
                mBubbleAImg.setDest(bubble.pos);
                tex.draw(mBubbleAImg);
                break;
            case BubbleType::B:
                mBubbleBImg.setDest(bubble.pos);
                tex.draw(mBubbleBImg);
                break;
        }
    }

    FireballBase::draw(tex);
}

const Number& Fireball::getPower() const { return mPower; }
void Fireball::setPower(const Number& pow) { mPower = pow; }

const Number& Fireball::getDuration() const { return mDuration; }
void Fireball::setDuration(const Number& duration) { mDuration = duration; }

void Fireball::applyTimeEffect(const Number& effect) { mPower ^= effect; }
}  // namespace PoisonWizard
