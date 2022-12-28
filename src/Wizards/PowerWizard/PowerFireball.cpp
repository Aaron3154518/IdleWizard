#include "PowerFireball.h"

namespace PowerWizard {
// Fireball
Fireball::Fireball(SDL_FPoint c, WizardId target, const FireballData& data)
    : FireballBase(c, target),
      mSizeSum(data.sizeFactor),
      mPower(data.power),
      mDuration(data.duration),
      mFromBot(data.fromBot) {
    setSize(data.sizeFactor);
    setSpeed(data.speed);
    setImg(IconSystem::Get(PowerWizard::Constants::FB_IMG()));
}

FireballData Fireball::getData() const {
    return {mPower, mDuration, mSize, mSpeed, mFromBot};
}

const Number& Fireball::getPower() const { return mPower; }
void Fireball::setPower(const Number& pow) { mPower = pow; }

const Number& Fireball::getDuration() const { return mDuration; }
void Fireball::setDuration(const Number& duration) { mDuration = duration; }

bool Fireball::isFromBot() const { return mFromBot; }

void Fireball::addFireball(const FireballData& data) {
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

bool Fireball::Fireball::filter(const PowerWizard::Fireball& fb, WizardId id) {
    if (fb.getTargetId() != id) {
        return false;
    }

    // If synergy is active, fireball must be from bot
    auto it = RobotWizard::Constants::SYN_TARGETS.find(id);
    if (it != RobotWizard::Constants::SYN_TARGETS.end() && it->second.get() &&
        !fb.isFromBot()) {
        return false;
    }

    return true;
}

void Fireball::applyTimeEffect(const Number& effect) { mPower ^= effect; }

// RobotFireballList
void RobotFireballList::init() {
    FireballListBase<Fireball>::init();

    // Don't use time system
    mUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            [this](Time dt) { onUpdate(dt); });
}
}  // namespace PowerWizard
