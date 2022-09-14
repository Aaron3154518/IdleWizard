#include "WizardFireball.h"

// WizardFireball
std::shared_ptr<WizardFireball::HitObservable>
WizardFireball::GetHitObservable() {
    return ServiceSystem::Get<Service, HitObservable>();
}

WizardFireball::WizardFireball(SDL_FPoint c, WizardId target, const Data& data)
    : Fireball(c, target, data.speed,
               data.boosted ? WizardDefs::FB_POW_IMG : WizardDefs::FB_IMG),
      mSizeSum(data.sizeFactor),
      mPower(data.power),
      mPowerWizBoosted(data.boosted) {
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
    float prevSizeFactor = fminf(powf(mSizeSum, 1.f / 3.f), 10);
    mSizeSum += data.sizeFactor;
    float sizeFactor = fminf(powf(mSizeSum, 1.f / 3.f), 10);
    setSize(mSize * sizeFactor / prevSizeFactor);

    if (data.boosted && !mPowerWizBoosted) {
        mPowerWizBoosted = true;
        mFireRingSub.reset();
        mImg.set(IconSystem::Get(WizardDefs::FB_POW_IMG));
    }
}

void WizardFireball::applyTimeEffect(const Number& effect) { mPower ^= effect; }
