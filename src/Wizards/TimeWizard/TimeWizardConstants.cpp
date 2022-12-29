#include "TimeWizardConstants.h"

namespace TimeWizard {
namespace Constants {
const AnimationData& IMG() {
    const static AnimationData IMG{"res/wizards/time_wizard_ss.png", 8, 100};
    return IMG;
}
const AnimationData& FREEZE_IMG() {
    const static AnimationData FREEZE_IMG{
        "res/wizards/time_wizard_frozen_ss.png", 9, 100};
    return FREEZE_IMG;
}

const std::string FREEZE_UP_IMG = "res/upgrades/time_freeze_upgrade.png",
                  SPEED_UP_IMG = "res/upgrades/speed_upgrade.png",
                  FB_SPEED_UP_IMG = "res/upgrades/fireball_speed_upgrade.png",
                  POW_SPEED_UP_IMG = "res/upgrades/power_speed_upgrade.png",
                  ACTIVE_UP_IMG = "res/upgrades/time_wizard_active.png";

void setDefaults() {
    using WizardSystem::Event;

    TimeWizard::Params params;

    params[TimeWizard::Param::SpeedBaseEffect]->init(1.5);
    params[TimeWizard::Param::FreezeBaseEffect]->init(1.1);
    params[TimeWizard::Param::FreezeDelay]->init(1000000);
    params[TimeWizard::Param::FreezeDuration]->init(15000);
    params[TimeWizard::Param::TimeWarpEffect]->init(1, Event::ResetT1);

    params[TimeWizard::Param::SpeedUpLvl]->init(Event::ResetT1);
    params[TimeWizard::Param::FBSpeedUpLvl]->init(Event::ResetT1);
    params[TimeWizard::Param::FreezeUpLvl]->init(Event::ResetT1);
    params[TimeWizard::Param::SpeedUpUpLvl]->init(Event::ResetT1);


    states[State::TimeWizActive]->init(false, Event::ResetT1);
    states[State::TimeWizFrozen]->init(false, Event::ResetT1);
}
}  // namespace Constants
}  // namespace TimeWizard
