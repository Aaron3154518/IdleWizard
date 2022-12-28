#ifndef WIZARD_FIREBALL_H
#define WIZARD_FIREBALL_H

#include <Components/FireballBase.h>
#include <Wizards/Catalyst/CatalystRing.h>
#include <Wizards/Crystal/FireRing.h>
#include <Wizards/Wizard/WizardConstants.h>

#include <random>

namespace Wizard {
class Fireball : public FireballBase {
    friend class FireballList;

   public:
    struct Data {
        Number power;
        float sizeFactor = 1, speed = 1;
        bool boosted = false, poisoned = false;
    };

    Fireball(SDL_FPoint c, WizardId target, const Data& data);

    void draw(TextureBuilder& tex);

    Data getData() const;

    bool isBoosted() const;
    bool isPoisoned() const;

    const Number& getPower() const;
    void setPower(const Number& pow);

    void addFireball(const Data& data);

    void applyTimeEffect(const Number& effect);

    void updateImage();

   private:
    void init();

    void onFireRingHit(const Number& effect);
    void onCatalystHit(bool poisoned);

    std::unique_ptr<Fireball> split();

    void setOnPoisoned(
        std::function<void(std::unique_ptr<Fireball>)> push_back);

    Number mPower;

    bool mHitFireRing = false, mBoosted = false, mPoisoned = false;

    float mSizeSum;

    RenderData mOuterImg;

    std::mt19937 gen = std::mt19937(rand());
    std::uniform_real_distribution<float> rDist;

    Crystal::FireRing::HitObservable::SubscriptionPtr mFireRingSub;
    Catalyst::Ring::HitObservable::SubscriptionPtr mCatalystHitSub;
    PoisonWizard::Glob::HitObservable::SubscriptionPtr mGlobHitSub;

    std::function<void()> mOnPoisoned;
};

typedef std::unique_ptr<Fireball> FireballPtr;

class FireballList : public FireballListBase<Fireball> {
   public:
    void push_back(FireballPtr fb);

   private:
};

typedef std::unique_ptr<FireballList> FireballListPtr;
}  // namespace Wizard

#endif
