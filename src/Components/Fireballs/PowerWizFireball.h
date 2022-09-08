#ifndef POWER_WIZ_FIREBALL_H
#define POWER_WIZ_FIREBALL_H

#include <Components/FireRing.h>
#include <Components/Fireballs/Fireball.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/TargetSystem.h>
#include <Utils/Number.h>

#include <memory>

class PowerWizFireball : public Fireball {
   public:
    struct Data {
        Number power, duration;
        float sizeFactor = 1, speed = .65;
        WizardId src = POWER_WIZARD;
    };

    typedef TargetSystem::TargetObservable<WizardId, const PowerWizFireball&>
        HitObservable;

    class Service : public ::Service<HitObservable> {};

    static std::shared_ptr<HitObservable> GetHitObservable();

    static RenderDataCWPtr GetIcon();

   public:
    PowerWizFireball(SDL_FPoint c, WizardId target, const Data& data);

    Data getData() const;

    const Number& getPower() const;
    void setPower(const Number& pow);

    const Number& getDuration() const;
    void setDuration(const Number& duration);

    void addFireball(const Data& data);

    void applyTimeEffect(const Number& effect);

    const static AnimationData IMG;

   private:
    void init();

    void onFireRingHit(const Number& effect);

    void onDeath();

    WizardId mSrc;
    int mFireballFreezeCnt = 1;
    float mSizeSum;
    Number mPower, mDuration;

    ParameterSystem::ParameterSubscriptionPtr mRobotBoughtSub;
};

typedef std::unique_ptr<PowerWizFireball> PowerWizFireballPtr;

#endif
