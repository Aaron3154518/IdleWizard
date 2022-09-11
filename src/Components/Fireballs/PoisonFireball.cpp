#include "PoisonFireball.h"

// PoisonFireball
std::shared_ptr<PoisonFireball::HitObservable>
PoisonFireball::GetHitObservable() {
    return ServiceSystem::Get<Service, HitObservable>();
}

PoisonFireball::PoisonFireball(SDL_FPoint c, WizardId target, const Data& data)
    : Fireball(c, target, PoisonWizardDefs::GLOB_IMG, data.speed),
      mSizeSum(data.sizeFactor),
      mPower(data.power),
      mDuration(data.duration) {
    setSize(data.sizeFactor);

    auto ptr = IconSystem::Get(PoisonWizardDefs::BUBBLE1_IMG).lock();
    if (ptr) {
        mBubbleAImg = *ptr;
    }
    ptr = IconSystem::Get(PoisonWizardDefs::BUBBLE2_IMG).lock();
    if (ptr) {
        mBubbleBImg = *ptr;
    }
}

void PoisonFireball::init() { Fireball::init(); }

void PoisonFireball::onUpdate(Time dt) {
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

    Fireball::onUpdate(dt);
}
void PoisonFireball::onRender(SDL_Renderer* renderer) {
    TextureBuilder tex;

    auto ptr = IconSystem::Get(PoisonWizardDefs::BUBBLE1_IMG).lock();
    if (ptr) {
        mBubbleAImg.setFrame(ptr->getFrame());
    }
    ptr = IconSystem::Get(PoisonWizardDefs::BUBBLE2_IMG).lock();
    if (ptr) {
        mBubbleBImg.setFrame(ptr->getFrame());
    }
    for (auto& bubble : mBubbles) {
        switch (bubble.type) {
            case BubbleType::A:
                tex.draw(mBubbleAImg.setDest(bubble.pos));
                break;
            case BubbleType::B:
                tex.draw(mBubbleBImg.setDest(bubble.pos));
                break;
        }
    }

    Fireball::onRender(renderer);
}

void PoisonFireball::onDeath() {
    GetHitObservable()->next(mTargetId, *this);

    mBubbles.clear();
}

const Number& PoisonFireball::getPower() const { return mPower; }
void PoisonFireball::setPower(const Number& pow) { mPower = pow; }

const Number& PoisonFireball::getDuration() const { return mDuration; }
void PoisonFireball::setDuration(const Number& duration) {
    mDuration = duration;
}

void PoisonFireball::applyTimeEffect(const Number& effect) { mPower ^= effect; }
