#include "PowerFireball.h"

// PowerFireball
std::shared_ptr<PowerFireball::HitObservable>
PowerFireball::GetHitObservable() {
    return ServiceSystem::Get<Service, HitObservable>();
}

PowerFireball::PowerFireball(SDL_FPoint c, WizardId target, const Data& data)
    : Fireball(c, target),
      mSrc(data.src),
      mTarget(target),
      mSizeSum(data.sizeFactor),
      mPower(data.power),
      mDuration(data.duration) {
    setSize(data.sizeFactor);
    setSpeed(data.speed);
    setImg(IconSystem::Get(PowerWizardDefs::FB_IMG));
}

void PowerFireball::init() {
    Fireball::init();

    switch (mSrc) {
        case POWER_WIZARD:
            mRobotBoughtSub = ParameterSystem::Param(State::ShootRobot)
                                  .subscribe([this](bool robot) {
                                      mTargetId =
                                          robot ? ROBOT_WIZARD : mTarget;
                                  });
            break;
    };
}

void PowerFireball::onDeath() { GetHitObservable()->next(mTargetId, *this); }

PowerFireball::Data PowerFireball::getData() const {
    return {mPower, mDuration, mSize, mSpeed};
}

WizardId PowerFireball::getTarget() const { return mTarget; }

const Number& PowerFireball::getPower() const { return mPower; }
void PowerFireball::setPower(const Number& pow) { mPower = pow; }

const Number& PowerFireball::getDuration() const { return mDuration; }
void PowerFireball::setDuration(const Number& duration) {
    mDuration = duration;
}

void PowerFireball::addFireball(const Data& data) {
    mFireballFreezeCnt++;
    mPower += data.power / mFireballFreezeCnt;
    switch (mTargetId) {
        case WIZARD:
            mDuration += data.duration / mFireballFreezeCnt;
            break;
    }
    float prevSizeFactor = fmin(pow(mSizeSum, 1.0 / 3.0), 10);
    mSizeSum += data.sizeFactor;
    float sizeFactor = fmin(pow(mSizeSum, 1.0 / 3.0), 10);
    setSize(mSize * sizeFactor / prevSizeFactor);
}

void PowerFireball::applyTimeEffect(const Number& effect) { mPower ^= effect; }
