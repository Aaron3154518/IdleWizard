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
    setImg(IconSystem::Get(PowerWizardDefs::FB_IMG()));
}

void PowerFireball::init() {
    Fireball::init();

    auto it = RobotWizardDefs::SYN_TARGETS.find(mTarget);
    if (it != RobotWizardDefs::SYN_TARGETS.end()) {
        mSynActiveSub =
            it->second.subscribe([this](bool active) { mCircle = active; });
    }

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

bool PowerFireball::onUpdate(Time dt) {
    if (!mCircle) {
        return Fireball::onUpdate(dt);
    }

    Rect target = GetSynergyBotPosObservable()->get(mTarget);
    float dx = target.cX() - mPos->rect.cX(),
          dy = target.cY() - mPos->rect.cY();
    float d = sqrtf(dx * dx + dy * dy);

    if (d <= COLLIDE_ERR) {
        onDeath();
        return false;
    }

    target = WizardSystem::GetWizardPos(mTargetId);
    dx = target.cX() - mPos->rect.cX();
    dy = target.cY() - mPos->rect.cY();
    d = sqrtf(dx * dx + dy * dy);
    float diag = sqrtf(powf(target.halfW(), 2) + powf(target.halfH(), 2));

    if (d >= diag) {
        return Fireball::onUpdate(dt);
    }

    // Fireball::onUpdate(dt);
    mTheta = fmodf(mTheta + dt.s() * M_PI / 4, 2 * M_PI);
    float tx = diag * cosf(mTheta) + target.cX(),
          ty = diag * sinf(mTheta) + target.cY();
    setPos(tx, ty);
    // dx = tx - mPos->rect.cX();
    // dy = ty - mPos->rect.cY();
    // d = sqrtf(dx * dx + dy * dy);
    // float maxSpeed = mSpeed * MAX_SPEED;

    return true;
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

// RobotFireballList
void RobotFireballList::init() {
    FireballList<PowerFireball>::init();

    // Don't use time system
    mUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            [this](Time dt) { onUpdate(dt); });
}
