#ifndef WIZARD_FIREBALL_H
#define WIZARD_FIREBALL_H

#include <Components/CatalystRing.h>
#include <Components/FireRing.h>
#include <Components/Fireballs/Fireball.h>
#include <Wizards/Definitions/WizardDefs.h>

class WizardFireball : public Fireball {
   public:
    typedef TargetSystem::TargetObservable<WizardId, const WizardFireball&>
        HitObservable;

    class Service : public ::Service<HitObservable> {};

    static std::shared_ptr<HitObservable> GetHitObservable();

   public:
    struct Data {
        Number power;
        float sizeFactor = 1, speed = 1;
        bool boosted = false;
    };

    WizardFireball(SDL_FPoint c, WizardId target, const Data& data);

    const Number& getPower() const;
    void setPower(const Number& pow);

    void addFireball(const Data& data);

    void applyTimeEffect(const Number& effect);

   private:
    void init();

    void onDeath();
    void onFireRingHit(const Number& effect);
    void onCatalystHit(const Number& effect);

    Number mPower;

    bool mHitFireRing = false, mPowerWizBoosted = false;

    float mSizeSum;

    FireRing::HitObservable::SubscriptionPtr mFireRingSub;
    CatalystRing::HitObservable::SubscriptionPtr mCatalystHitSub;
};

typedef std::unique_ptr<WizardFireball> WizardFireballPtr;

#endif
