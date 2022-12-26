#include "PowerFireball.h"

// PowerFireball
std::shared_ptr<PowerFireball::HitObservable>
PowerFireball::GetHitObservable() {
    return ServiceSystem::Get<Service, HitObservable>();
}

PowerFireball::PowerFireball(SDL_FPoint c, WizardId target,
                             const PowerFireballData& data)
    : Fireball(c, target),
      mSizeSum(data.sizeFactor),
      mPower(data.power),
      mDuration(data.duration) {
    setSize(data.sizeFactor);
    setSpeed(data.speed);
    setImg(IconSystem::Get(PowerWizardDefs::FB_IMG()));
}

void PowerFireball::init() {
    Fireball::init();

    mSynBotHitSub = GetSynergyBotHitObservable()->subscribeFb(
        mTargetId,
        [this]() -> PowerFireballData {
            // TODO: Better separation of different deaths
            Fireball::onDeath();
            return getData();
        },
        mPos);
    mSynBotHitSub->setActive(false);

    auto it = RobotWizardDefs::SYN_TARGETS.find(mTargetId);
    if (it != RobotWizardDefs::SYN_TARGETS.end()) {
        mSynActiveSub = it->second.subscribe(
            [this](bool active) { mSynBotHitSub->setActive(active); });
    }
}

void PowerFireball::onUpdate(Time dt) {
    if (!mSynBotHitSub->isActive()) {
        Fireball::onUpdate(dt);
        return;
    }

    Rect target = WizardSystem::GetWizardPos(mTargetId);
    float dx = target.cX() - mPos->rect.cX(),
          dy = target.cY() - mPos->rect.cY();
    float d = sqrtf(dx * dx + dy * dy);
    float diag = sqrtf(powf(target.halfW(), 2) + powf(target.halfH(), 2));

    if (d >= diag) {
        Fireball::onUpdate(dt);
        return;
    }

    move(dt);
}

void PowerFireball::onDeath() {
    Fireball::onDeath();

    GetHitObservable()->next(mTargetId, *this);
}

PowerFireballData PowerFireball::getData() const {
    return {mPower, mDuration, mSize, mSpeed};
}

WizardId PowerFireball::getTarget() const { return mTargetId; }

const Number& PowerFireball::getPower() const { return mPower; }
void PowerFireball::setPower(const Number& pow) { mPower = pow; }

const Number& PowerFireball::getDuration() const { return mDuration; }
void PowerFireball::setDuration(const Number& duration) {
    mDuration = duration;
}

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
