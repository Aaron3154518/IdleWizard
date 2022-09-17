#include "WizardFireball.h"

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
      mPoisoned(data.poisoned) {
    setSize(data.sizeFactor);
    setSpeed(data.speed);
    setInnerImg(IconSystem::Get(data.poisoned ? WizardDefs::FB_INNER_POISON_IMG
                                              : WizardDefs::FB_INNER_IMG));
    setOuterImg(IconSystem::Get(data.boosted ? WizardDefs::FB_OUTER_BUFFED_IMG
                                             : WizardDefs::FB_OUTER_IMG));
}

void WizardFireball::init() {
    Fireball::init();

    if (!mBoosted) {
        mFireRingSub = FireRing::GetHitObservable()->subscribe(
            [this](const Number& e) { onFireRingHit(e); }, mPos);
    }
    mCatalystHitSub = CatalystRing::GetHitObservable()->subscribe(
        [this](const Number& e) { onCatalystHit(e); }, mPos);
}

void WizardFireball::onRender(TextureBuilder& tex) {
    Fireball::onRender(tex);
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

void WizardFireball::onCatalystHit(const Number& effect) {
    mPower *= effect;

    setSize(mSize * 1.1);
}

void WizardFireball::subscribeToGlob(
    std::function<void(WizardFireballPtr)> push_back) {
    mGlobHitSub = Glob::GetHitObservable()->subscribe(
        [this, push_back]() {
            if (!mPoisoned) {
                mPoisoned = true;
                setInnerImg(IconSystem::Get(WizardDefs::FB_INNER_POISON_IMG));
            } else {
                push_back(clone());
            }
        },
        mPos);
}

std::unique_ptr<WizardFireball> WizardFireball::clone() const {
    return ComponentFactory<WizardFireball>::New(
        mPos->rect.getPos(Rect::Align::CENTER), mTargetId, getData());
}

WizardFireball::Data WizardFireball::getData() const {
    return {mPower, mSize, mSpeed, mBoosted, mPoisoned};
}

const Number& WizardFireball::getPower() const { return mPower; }
void WizardFireball::setPower(const Number& pow) { mPower = pow; }

void WizardFireball::setInnerImg(const RenderTextureCPtr& img) {
    Fireball::setImg(img);
}

void WizardFireball::setOuterImg(const RenderTextureCPtr& img) {
    mOuterImg.set(img);
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
        setOuterImg(IconSystem::Get(WizardDefs::FB_OUTER_BUFFED_IMG));
    }

    if (data.poisoned && !mPoisoned) {
        mPoisoned = true;
        setInnerImg(IconSystem::Get(WizardDefs::FB_INNER_POISON_IMG));
    }
}

void WizardFireball::applyTimeEffect(const Number& effect) { mPower ^= effect; }

// WizardFireballList
void WizardFireballList::push_back(WizardFireballPtr fb) {
    fb->subscribeToGlob(
        [this](WizardFireballPtr fb) { push_back(std::move(fb)); });
    FireballList::push_back(std::move(fb));
}