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
#include <Wizards/Definitions/PowerWizardDefs.h>

#include <memory>

class PowerFireball : public Fireball {
   public:
    typedef TargetSystem::TargetObservable<WizardId, const PowerFireball&>
        HitObservable;

    class Service : public ::Service<HitObservable> {};

    static std::shared_ptr<HitObservable> GetHitObservable();

    static RenderTextureCPtr GetIcon();

   public:
    struct Data {
        Number power, duration;
        float sizeFactor = 1, speed = .65;
        WizardId src = POWER_WIZARD;
    };

    PowerFireball(SDL_FPoint c, WizardId target, const Data& data);

    Data getData() const;

    const Number& getPower() const;
    void setPower(const Number& pow);

    const Number& getDuration() const;
    void setDuration(const Number& duration);

    WizardId getTarget() const;

    void addFireball(const Data& data);

    void applyTimeEffect(const Number& effect);

   private:
    void init();

    void onFireRingHit(const Number& effect);

    void onDeath();

    // mTarget = actual target
    WizardId mSrc, mTarget;
    int mFireballFreezeCnt = 1;
    float mSizeSum;
    Number mPower, mDuration;

    ParameterSystem::ParameterSubscriptionPtr mRobotBoughtSub;
};

typedef std::unique_ptr<PowerFireball> PowerFireballPtr;

#endif
