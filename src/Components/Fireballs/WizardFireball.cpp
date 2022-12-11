#include "WizardFireball.h"

#include <Systems/WizardSystem/MagicObservables.h>

// WizardFireball
std::shared_ptr<WizardFireball::HitObservable>
WizardFireball::GetHitObservable() {
    return ServiceSystem::Get<Service, HitObservable>();
}

WizardFireball::WizardFireball(SDL_FPoint c, WizardId target, const Data& data)
    : Fireball(c, target),
      mSizeSum(data.sizeFactor),
      mPower(data.power),
      mBoosted(data.boosted),
      mPoisoned(data.poisoned),
      mOnPoisoned([]() {}) {
    setSize(data.sizeFactor);
    setSpeed(data.speed);
    updateImage();
}

void WizardFireball::init() {
    Fireball::init();

    if (!mBoosted) {
        mFireRingSub = FireRing::GetHitObservable()->subscribe(
            [this](const Number& e) { onFireRingHit(e); }, mPos);
    }
    mCatalystHitSub = CatalystRing::GetHitObservable()->subscribe(
        [this](bool b) { onCatalystHit(b); }, mPos);
    mGlobHitSub =
        Glob::GetHitObservable()->subscribe([this]() { mOnPoisoned(); }, mPos);
}

void WizardFireball::draw(TextureBuilder& tex) {
    Fireball::draw(tex);
    mOuterImg.setDest(mImg.getRect());
    mOuterImg.setRotationDeg(mImg.getRotationDeg());
    tex.draw(mOuterImg);
}

void WizardFireball::onDeath() {
    mFireRingSub.reset();
    mCatalystHitSub.reset();

    GetHitObservable()->next(mTargetId, *this);
}

void WizardFireball::onFireRingHit(const Number& effect) {
    mPower *= effect;
    mHitFireRing = true;
    mFireRingSub.reset();

    setSize(mSize * 1.15);
}

void WizardFireball::onCatalystHit(bool poisoned) {
    WizardSystem::GetCatalystMagicObservable()->next(*this);
    if (poisoned) {
        mOnPoisoned();
    }
}

void WizardFireball::setOnPoisoned(
    std::function<void(WizardFireballPtr)> push_back) {
    mOnPoisoned = [this, push_back]() {
        if (!mPoisoned) {
            mPoisoned = true;
            updateImage();
        } else {
            push_back(split());
        }
    };
}

std::unique_ptr<WizardFireball> WizardFireball::split() {
    SDL_FPoint c = mPos->rect.getPos(Rect::Align::CENTER);
    float theta = rDist(gen) * 2 * M_PI;
    float r = (rDist(gen) * .25 + .25) * mPos->rect.minDim();
    float dx = r * cosf(theta), dy = r * sinf(theta);

    auto data = getData();
    data.poisoned = false;
    auto fb = ComponentFactory<WizardFireball>::New(
        SDL_FPoint{c.x + dx, c.y + dy}, mTargetId, data);
    fb->launch(SDL_FPoint{c.x + dx + dx, c.y + dy + dy});

    return fb;
}

WizardFireball::Data WizardFireball::getData() const {
    return {mPower, mSize, mSpeed, mBoosted, mPoisoned};
}

bool WizardFireball::isBoosted() const { return mBoosted; }
bool WizardFireball::isPoisoned() const { return mPoisoned; }

const Number& WizardFireball::getPower() const { return mPower; }
void WizardFireball::setPower(const Number& pow) { mPower = pow; }

void WizardFireball::updateImage() {
    Fireball::setImg(IconSystem::Get(mPoisoned ? WizardDefs::FB_INNER_POISON_IMG
                                               : WizardDefs::FB_INNER_IMG));
    mOuterImg.set(IconSystem::Get(mBoosted ? WizardDefs::FB_OUTER_BUFFED_IMG
                                           : WizardDefs::FB_OUTER_IMG));
}

void WizardFireball::addFireball(const Data& data) {
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

void WizardFireball::applyTimeEffect(const Number& effect) { mPower ^= effect; }

// WizardFireballList
void WizardFireballList::push_back(WizardFireballPtr fb) {
    fb->setOnPoisoned(
        [this](WizardFireballPtr fb) { push_back(std::move(fb)); });
    FireballList::push_back(std::move(fb));
}