#include "WizardFireball.h"

// WizardFireball
const AnimationData WizardFireball::IMG{"res/projectiles/fireball_ss.png", 6,
                                        75},
    WizardFireball::POW_IMG{"res/projectiles/fireball_buffed_ss.png", 6, 75};

RenderDataWPtr WizardFireball::GetIcon() {
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

std::shared_ptr<WizardFireball::HitObservable>
WizardFireball::GetHitObservable() {
    return ServiceSystem::Get<Service, HitObservable>();
}

WizardFireball::WizardFireball(SDL_FPoint c, WizardId target, const Data& data,
                               bool powerWizBoosted)
    : Fireball(c, target, powerWizBoosted ? POW_IMG : IMG, data.speed),
      mSizeSum(data.sizeFactor),
      mPower(data.power),
      mPowerWizBoosted(powerWizBoosted) {
    setSize(data.sizeFactor);
}

void WizardFireball::init() {
    Fireball::init();

    if (!mPowerWizBoosted) {
        mFireRingSub = FireRing::GetHitObservable()->subscribe(
            [this](const Number& e) { onFireRingHit(e); }, mPos);
    }
    mCatalystHitSub = CatalystRing::GetHitObservable()->subscribe(
        [this](const Number& e) { onCatalystHit(e); }, mPos);
}

void WizardFireball::onFireRingHit(const Number& effect) {
    mPower *= effect;
    mHitFireRing = true;
    mFireRingSub.reset();

    setSize(mSize * 1.15);
}

void WizardFireball::onCatalystHit(const Number& effect) {
    mPower *= effect;

    setSize(mSize * 1.1);
}

void WizardFireball::onDeath() {
    mFireRingSub.reset();
    mCatalystHitSub.reset();

    GetHitObservable()->next(mTargetId, *this);
}

const Number& WizardFireball::getPower() const { return mPower; }
void WizardFireball::setPower(const Number& pow) { mPower = pow; }

void WizardFireball::addFireball(const Data& data) {
    mPower += data.power;
    float prevSizeFactor = fmin(pow(mSizeSum, 1.0 / 3.0), 10);
    mSizeSum += data.sizeFactor;
    float sizeFactor = fmin(pow(mSizeSum, 1.0 / 3.0), 10);
    setSize(mSize * sizeFactor / prevSizeFactor);
}

void WizardFireball::applyTimeEffect(const Number& effect) { mPower ^= effect; }
