#ifndef POWER_WIZ_FIREBALL_H
#define POWER_WIZ_FIREBALL_H

#include <Components/FireRing.h>
#include <Components/Fireballs/Fireball.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/TargetSystem.h>
#include <Utils/Number.h>

#include <memory>

class PowerWizFireball : public Fireball {
   public:
    struct Data {
        Number power, duration;
        int sizeFactor = 1;
        float speed = .65;
    };

    typedef TargetSystem::TargetObservable<WizardId, const PowerWizFireball&>
        HitObservable;

    class Service : public ::Service<HitObservable> {};

    static std::shared_ptr<HitObservable> GetHitObservable();

    static const RenderDataPtr& GetIcon();

   public:
    PowerWizFireball(SDL_FPoint c, WizardId target, const Data& data);

    Number power() const;
    Number duration() const;

    void addFireball(const Data& data);

    void applyTimeEffect(const Number& effect);

    const static AnimationData IMG;

   private:
    void init();

    void onFireRingHit(const Number& effect);

    void onDeath();

    int mFireballFreezeCnt = 1, mSizeSum = 0;
    Number mPower = 0, mDuration = 0;
};

typedef std::unique_ptr<PowerWizFireball> PowerWizFireballPtr;

#endif
