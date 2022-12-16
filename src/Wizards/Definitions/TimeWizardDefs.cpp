#include "TimeWizardDefs.h"

namespace TimeWizardDefs {
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

    ParameterSystem::Params<TIME_WIZARD> params;

    params[TimeWizardParams::SpeedBaseEffect]->init(1.5);
    params[TimeWizardParams::FreezeBaseEffect]->init(1.1);
    params[TimeWizardParams::FreezeDelay]->init(1000000);
    params[TimeWizardParams::FreezeDuration]->init(15000);
    params[TimeWizardParams::TimeWarpEffect]->init(1, Event::ResetT1);

    params[TimeWizardParams::SpeedUpLvl]->init(Event::ResetT1);
    params[TimeWizardParams::FBSpeedUpLvl]->init(Event::ResetT1);
    params[TimeWizardParams::FreezeUpLvl]->init(Event::ResetT1);
    params[TimeWizardParams::SpeedUpUpLvl]->init(Event::ResetT1);

    ParameterSystem::States states;

    states[State::TimeWizActive]->init(false, Event::ResetT1);
    states[State::TimeWizFrozen]->init(false, Event::ResetT1);
}
}  // namespace TimeWizardDefs
