#include "RobotWizardDefs.h"

namespace RobotWizardDefs {
const std::vector<WizardId> TARGETS{CRYSTAL, WIZARD, POWER_WIZARD, TIME_WIZARD};

const AnimationData IMG{"res/wizards/robot_ss.png", 6, 100},
    PORTAL_TOP{"res/wizards/portal_top.png", 6, 150},
    PORTAL_BOT{"res/wizards/portal_bottom.png", 6, 150};

void setDefaults() {
    using WizardSystem::Event;

    ParameterSystem::Params<CRYSTAL> params;

    ParameterSystem::States states;
}
}  // namespace RobotWizardDefs
