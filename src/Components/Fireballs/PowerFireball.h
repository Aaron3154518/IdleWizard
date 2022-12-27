#ifndef POWER_WIZ_FIREBALL_H
#define POWER_WIZ_FIREBALL_H

// #include <Components/Bot.h>
#include <Components/FireRing.h>
#include <Components/Fireballs/Fireball.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/TargetSystem.h>
#include <Utils/Number.h>
#include <Wizards/Definitions/PowerWizardDefs.h>
#include <Wizards/Definitions/RobotWizardDefs.h>

#include <memory>

struct PowerFireballData {
    Number power, duration;
    float sizeFactor = 1, speed = .65;
    bool fromBot = false;
};

class PowerFireball : public Fireball {
   public:
    typedef TargetSystem::TargetObservable<WizardId, const PowerFireball&>
        HitObservable;

    class Service : public ::Service<HitObservable> {};

    static std::shared_ptr<HitObservable> GetHitObservable();

   public:
    PowerFireball(SDL_FPoint c, WizardId target, const PowerFireballData& data);

    PowerFireballData getData() const;

    const Number& getPower() const;
    void setPower(const Number& pow);

    const Number& getDuration() const;
    void setDuration(const Number& duration);

    bool isFromBot() const;

    void addFireball(const PowerFireballData& data);

    void applyTimeEffect(const Number& effect);

   private:
    void init();

    void onUpdate(Time dt);

    void onDeath();

    const bool mFromBot;
    int mFireballFreezeCnt = 1;
    float mSizeSum;
    Number mPower, mDuration;

    ParameterSystem::ParameterSubscriptionPtr mRobotBoughtSub, mSynActiveSub;
    // SynergyBot::HitObservable::FbSubscriptionPtr mSynBotHitSub;
};

typedef std::unique_ptr<PowerFireball> PowerFireballPtr;

typedef FireballList<PowerFireball> PowerFireballList;
typedef std::unique_ptr<PowerFireballList> PowerFireballListPtr;

class RobotFireballList : public PowerFireballList {
   private:
    void init();
};

typedef std::unique_ptr<RobotFireballList> RobotFireballListPtr;

#endif
