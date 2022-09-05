#ifndef WIZARD_FIREBALL_H
#define WIZARD_FIREBALL_H

#include <Components/CatalystRing.h>
#include <Components/FireRing.h>
#include <Components/Fireballs/Fireball.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/TargetSystem.h>
#include <Utils/Number.h>

#include <memory>

class WizardFireball : public Fireball {
   public:
    struct Data {
        Number power;
        float sizeFactor = 1, speed = 1;
    };

    typedef TargetSystem::TargetObservable<WizardId, const WizardFireball&>
        HitObservable;

    class Service : public ::Service<HitObservable> {};

    static std::shared_ptr<HitObservable> GetHitObservable();

   public:
    WizardFireball(SDL_FPoint c, WizardId target, const Data& data,
                   bool powerWizBoosted = false);

    const Number& getPower() const;
    void setPower(const Number& pow);

    void addFireball(const Data& data);

    void applyTimeEffect(const Number& effect);

    const static AnimationData IMG, POW_IMG;

    static RenderDataWPtr GetIcon();

   private:
    void init();

    void onFireRingHit(const Number& effect);
    void onCatalystHit(const Number& effect);

    void onDeath();

    bool mHitFireRing = false, mPowerWizBoosted = false;

    int mSizeSum = 0;
    Number mPower = 0;

    FireRing::HitObservable::SubscriptionPtr mFireRingSub;
    CatalystRing::HitObservable::SubscriptionPtr mCatalystHitSub;
};

typedef std::unique_ptr<WizardFireball> WizardFireballPtr;

#endif
