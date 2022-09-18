#include "RobotWizardDefs.h"

namespace RobotWizardDefs {
const std::vector<WizardId> TARGETS{CRYSTAL, WIZARD, POWER_WIZARD, TIME_WIZARD};

const AnimationData IMG{"res/wizards/robot_ss.png", 6, 100},
    PORTAL_TOP{"res/wizards/portal_top.png", 6, 150},
    PORTAL_BOT{"res/wizards/portal_bottom.png", 6, 150};

void setDefaults() {
    using WizardSystem::Event;

    ParameterSystem::Params<ROBOT_WIZARD> params;

    params[RobotWizardParams::WizCritUpCost]->init(Number(1, 20));

    ParameterSystem::States states;
}
}  // namespace RobotWizardDefs
