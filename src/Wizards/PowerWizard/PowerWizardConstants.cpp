#include "PowerWizardConstants.h"

namespace PowerWizard {
namespace Constants {
const AnimationData& IMG() {
    const static AnimationData IMG{"res/wizards/power_wizard_ss.png", 8, 150};
    return IMG;
}
const AnimationData& FB_IMG() {
    const static AnimationData FB_IMG{"res/projectiles/power_fireball_ss.png",
                                      6, 75};
    return FB_IMG;
}

const std::string POWER_UP_IMG = "res/upgrades/power_fireball_upgrade.png",
                  TIME_WARP_UP_IMG = "res/upgrades/time_warp_upgrade.png";

void setDefaults() {
    using WizardSystem::Event;

    Params params;

    params[Param::BasePower]->init(5);
    params[Param::BaseSpeed]->init(.25);
    params[Param::BaseFBSpeed]->init(.75);

    params[Param::PowerUpLvl]->init(Event::ResetT1);
    params[Param::TimeWarpUpLvl]->init(Event::ResetT1);
}
}  // namespace Constants
}  // namespace PowerWizard
