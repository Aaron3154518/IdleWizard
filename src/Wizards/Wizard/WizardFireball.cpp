#include "WizardFireball.h"

#include <Systems/WizardSystem/MagicObservables.h>

namespace Wizard {
// Fireball
Fireball::Fireball(SDL_FPoint c, WizardId target, const Data& data)
    : FireballBase(c, target),
      mSizeSum(data.sizeFactor),
      mPower(data.power),
      mBoosted(data.boosted),
      mPoisoned(data.poisoned),
      mOnPoisoned([]() {}) {
    setSize(data.sizeFactor);
    setSpeed(data.speed);
    updateImage();
}

void Fireball::init() {
    FireballBase::init();

    if (!mBoosted) {
        mFireRingSub = Crystal::FireRing::GetHitObservable()->subscribe(
            [this](const Number& e) { onFireRingHit(e); }, mPos);
    }
    mCatalystHitSub = Catalyst::Ring::GetHitObservable()->subscribe(
        [this](bool b) { onCatalystHit(b); }, mPos);
    mGlobHitSub =
        PoisonWizard::Glob::GetHitObservable()->subscribe([this]() { mOnPoisoned(); }, mPos);
}

void Fireball::draw(TextureBuilder& tex) {
    FireballBase::draw(tex);
    mOuterImg.setDest(mImg.getRect());
    mOuterImg.setRotationDeg(mImg.getRotationDeg());
    tex.draw(mOuterImg);
}

void Fireball::onFireRingHit(const Number& effect) {
    mPower *= effect;
    mHitFireRing = true;
    mFireRingSub.reset();

    setSize(mSize * 1.15);
}

void Fireball::onCatalystHit(bool poisoned) {
    WizardSystem::GetCatalystMagicObservable()->next(*this);
    if (poisoned) {
        mOnPoisoned();
    }
}

void Fireball::setOnPoisoned(std::function<void(FireballPtr)> push_back) {
    mOnPoisoned = [this, push_back]() {
        if (!mPoisoned) {
            mPoisoned = true;
            updateImage();
        } else {
            push_back(split());
        }
    };
}

std::unique_ptr<Fireball> Fireball::split() {
    SDL_FPoint c = mPos->rect.getPos(Rect::Align::CENTER);
    float theta = rDist(gen) * 2 * M_PI;
    float r = (rDist(gen) * .25 + .25) * mPos->rect.minDim();
    float dx = r * cosf(theta), dy = r * sinf(theta);

    auto data = getData();
    data.poisoned = false;
    auto fb = ComponentFactory<Fireball>::New(SDL_FPoint{c.x + dx, c.y + dy},
                                              mTargetId, data);
    fb->launch(SDL_FPoint{c.x + dx + dx, c.y + dy + dy});

    return fb;
}

Fireball::Data Fireball::getData() const {
    return {mPower, mSize, mSpeed, mBoosted, mPoisoned};
}

bool Fireball::isBoosted() const { return mBoosted; }
bool Fireball::isPoisoned() const { return mPoisoned; }

const Number& Fireball::getPower() const { return mPower; }
void Fireball::setPower(const Number& pow) { mPower = pow; }

void Fireball::updateImage() {
    FireballBase::setImg(IconSystem::Get(mPoisoned
                                             ? Wizard::Constants::FB_INNER_POISON_IMG()
                                             : Wizard::Constants::FB_INNER_IMG()));
    mOuterImg.set(IconSystem::Get(mBoosted ? Wizard::Constants::FB_OUTER_BUFFED_IMG()
                                           : Wizard::Constants::FB_OUTER_IMG()));
}

void Fireball::addFireball(const Data& data) {
    mPower += data.power;
    float prevSizeFactor = fminf(powf(mSizeSum, 1.f / 3.f), 10);
    mSizeSum += data.sizeFactor;
    float sizeFactor = fminf(powf(mSizeSum, 1.f / 3.f), 10);
    setSize(mSize * sizeFactor / prevSizeFactor);

    if (data.boosted && !mBoosted) {
        mBoosted = true;
        mFireRingSub.reset();
        updateImage();
    }

    if (data.poisoned && !mPoisoned) {
        mPoisoned = true;
        updateImage();
    }
}

void Fireball::applyTimeEffect(const Number& effect) { mPower ^= effect; }

// FireballList
void FireballList::push_back(FireballPtr fb) {
    fb->setOnPoisoned([this](FireballPtr fb) { push_back(std::move(fb)); });
    FireballList::push_back(std::move(fb));
}
}