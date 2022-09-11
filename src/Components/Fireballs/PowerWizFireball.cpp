#include "PowerWizFireball.h"

// PowerWizFireball
std::shared_ptr<PowerWizFireball::HitObservable>
PowerWizFireball::GetHitObservable() {
    return ServiceSystem::Get<Service, HitObservable>();
}

PowerWizFireball::PowerWizFireball(SDL_FPoint c, WizardId target,
                                   const Data& data)
    : Fireball(c, target, PowerWizardDefs::FB_IMG, data.speed),
      mSrc(data.src),
      mSizeSum(data.sizeFactor),
      mPower(data.power),
      mDuration(data.duration) {
    setSize(data.sizeFactor);
}

void PowerWizFireball::init() {
    Fireball::init();

    switch (mSrc) {
        case POWER_WIZARD:
            mRobotBoughtSub =
                ParameterSystem::Param(State::ShootRobot)
                    .subscribe([this](bool robot) {
                        mTargetSub =
                            WizardSystem::GetWizardPosObservable()->subscribe(
                                [this](const Rect& r) {
                                    mTargetPos = r.getPos(Rect::Align::CENTER);
                                },
                                robot ? ROBOT_WIZARD : mTargetId);
                    });
            break;
        case ROBOT_WIZARD:
            mUpdateSub = ServiceSystem::Get<UpdateService, UpdateObservable>()
                             ->subscribe([this](Time dt) { onUpdate(dt); });
            break;
    };
}

void PowerWizFireball::onDeath() {
    GetHitObservable()->next(
        mTargetSub->get<WizardSystem::WizardPosObservable::ID>(), *this);
}

PowerWizFireball::Data PowerWizFireball::getData() const {
    return {mPower, mDuration, mSize, mMaxSpeed / MAX_SPEED};
}

const Number& PowerWizFireball::getPower() const { return mPower; }
void PowerWizFireball::setPower(const Number& pow) { mPower = pow; }

const Number& PowerWizFireball::getDuration() const { return mDuration; }
void PowerWizFireball::setDuration(const Number& duration) {
    mDuration = duration;
}

void PowerWizFireball::addFireball(const Data& data) {
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

void PowerWizFireball::applyTimeEffect(const Number& effect) {
    mPower ^= effect;
}
