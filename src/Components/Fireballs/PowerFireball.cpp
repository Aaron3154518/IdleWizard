#include "PowerFireball.h"

// PowerFireball
PowerFireball::PowerFireball(SDL_FPoint c, WizardId target,
                             const PowerFireballData& data)
    : Fireball(c, target),
      mSizeSum(data.sizeFactor),
      mPower(data.power),
      mDuration(data.duration),
      mFromBot(data.fromBot) {
    setSize(data.sizeFactor);
    setSpeed(data.speed);
    setImg(IconSystem::Get(PowerWizardDefs::FB_IMG()));
}

PowerFireballData PowerFireball::getData() const {
    return {mPower, mDuration, mSize, mSpeed, mFromBot};
}

const Number& PowerFireball::getPower() const { return mPower; }
void PowerFireball::setPower(const Number& pow) { mPower = pow; }

const Number& PowerFireball::getDuration() const { return mDuration; }
void PowerFireball::setDuration(const Number& duration) {
    mDuration = duration;
}

bool PowerFireball::isFromBot() const { return mFromBot; }

void PowerFireball::addFireball(const PowerFireballData& data) {
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

// RobotFireballList
void RobotFireballList::init() {
    FireballList<PowerFireball>::init();

    // Don't use time system
    mUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            [this](Time dt) { onUpdate(dt); });
}
