#include "PoisonFireball.h"

// PoisonFireball
std::shared_ptr<PoisonFireball::HitObservable>
PoisonFireball::GetHitObservable() {
    return ServiceSystem::Get<Service, HitObservable>();
}

PoisonFireball::PoisonFireball(SDL_FPoint c, WizardId target, const Data& data)
    : Fireball(c, target, PoisonWizardDefs::GLOB_IMG, data.speed),
      mSizeSum(data.sizeFactor),
      mPower(data.power) {
    setSize(data.sizeFactor);
}

void PoisonFireball::init() { Fireball::init(); }

void PoisonFireball::onDeath() { GetHitObservable()->next(mTargetId, *this); }

const Number& PoisonFireball::getPower() const { return mPower; }
void PoisonFireball::setPower(const Number& pow) { mPower = pow; }

void PoisonFireball::applyTimeEffect(const Number& effect) { mPower ^= effect; }
