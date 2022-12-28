#ifndef POWER_WIZ_FIREBALL_H
#define POWER_WIZ_FIREBALL_H

// #include <Wizards/RobotWizard/Bot.h>
#include <Wizards/Crystal/FireRing.h>
#include <Components/FireballBase.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/TargetSystem.h>
#include <Utils/Number.h>
#include <Wizards/PowerWizard/PowerWizardConstants.h>
#include <Wizards/RobotWizard/RobotWizardConstants.h>

#include <memory>

namespace PowerWizard {
struct FireballData {
    Number power, duration;
    float sizeFactor = 1, speed = .65;
    bool fromBot = false;
};

class Fireball : public FireballBase {
   public:
    Fireball(SDL_FPoint c, WizardId target, const FireballData& data);

    FireballData getData() const;

    const Number& getPower() const;
    void setPower(const Number& pow);

    const Number& getDuration() const;
    void setDuration(const Number& duration);

    bool isFromBot() const;

    void addFireball(const FireballData& data);

    void applyTimeEffect(const Number& effect);

    static bool filter(const PowerWizard::Fireball& fb, WizardId id);

   private:
    const bool mFromBot;
    int mFireballFreezeCnt = 1;
    float mSizeSum;
    Number mPower, mDuration;

    ParameterSystem::ParameterSubscriptionPtr mRobotBoughtSub, mSynActiveSub;
    // SynergyBot::HitObservable::FbSubscriptionPtr mSynBotHitSub;
};

typedef std::unique_ptr<Fireball> FireballPtr;

typedef FireballListBase<Fireball> FireballList;
typedef std::unique_ptr<FireballList> FireballListPtr;

class RobotFireballList : public FireballList {
   private:
    void init();
};

typedef std::unique_ptr<RobotFireballList> RobotFireballListPtr;
}  // namespace PowerWizard

#endif
