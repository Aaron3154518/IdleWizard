#include "PowerWizFireball.h"

// PowerWizFireball
const AnimationData PowerWizFireball::IMG{
    "res/projectiles/power_fireball_ss.png", 6, 75};

const RenderDataPtr& PowerWizFireball::GetIcon() {
    static RenderDataPtr ICON;
    static TimerObservable::SubscriptionPtr ANIM_SUB;
    if (!ICON) {
        ICON = std::make_shared<RenderData>();
        ICON->set(IMG);
        ANIM_SUB =
            ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
                [](Timer& t) {
                    ICON->nextFrame();
                    return true;
                },
                Timer(IMG.frame_ms));
    }

    return ICON;
}

std::shared_ptr<PowerWizFireball::HitObservable>
PowerWizFireball::GetHitObservable() {
    return ServiceSystem::Get<Service, HitObservable>();
}

PowerWizFireball::PowerWizFireball(SDL_FPoint c, WizardId target,
                                   const Data& data)
    : Fireball(c, target, IMG, data.speed),
      mSizeSum(data.sizeFactor),
      mPower(data.power),
      mDuration(data.duration) {
    setSize(data.sizeFactor);
}

void PowerWizFireball::init() { Fireball::init(); }

void PowerWizFireball::onDeath() { GetHitObservable()->next(mTargetId, *this); }

Number PowerWizFireball::power() const { return mPower; }

Number PowerWizFireball::duration() const { return mDuration; }

void PowerWizFireball::addFireball(const Data& data) {
    mFireballFreezeCnt++;
    switch (mTargetId) {
        case WIZARD:
            mDuration += data.duration / sqrt(mFireballFreezeCnt);
            break;
        case CRYSTAL:
            mPower += data.power / sqrt(mFireballFreezeCnt) / 10;
            break;
    }
    float prevSizeFactor = fmin(pow(mSizeSum, 1.0 / 3.0), 10);
    mSizeSum += data.sizeFactor;
    float sizeFactor = fmin(pow(mSizeSum, 1.0 / 3.0), 10);
    setSize(mSize * sizeFactor / prevSizeFactor);
}

void PowerWizFireball::applyTimeEffect(const Number& effect) {
    switch (mTargetId) {
        case WIZARD:
            mDuration = ((mDuration / 1000) ^ effect) * 1000;
            break;
        case CRYSTAL:
            mPower ^= effect;
            break;
    }
}
