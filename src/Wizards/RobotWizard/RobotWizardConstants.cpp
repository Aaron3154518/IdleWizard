#include <Components/FireballBase.h>
#include <Wizards/PowerWizard/PowerFireball.h>

#include "RobotWizardConstants.h"

namespace RobotWizard {
namespace Constants {
const std::vector<WizardId> UP_TARGETS{CRYSTAL, WIZARD, POWER_WIZARD,
                                       TIME_WIZARD};
const std::unordered_map<WizardId, ParameterSystem::StateParam> SYN_TARGETS = {
    {CRYSTAL, ParameterSystem::Param(State::CrysSynBotActive)},
    {WIZARD, ParameterSystem::Param(State::WizSynBotActive)},
    {TIME_WIZARD, ParameterSystem::Param(State::TimeWizSynBotActive)},
};

const AnimationData& IMG() {
    const static AnimationData IMG{"res/wizards/robot_ss.png", 6, 100};
    return IMG;
}
const AnimationData& UP_BOT_IMG() {
    const static AnimationData UP_BOT_IMG{"res/wizards/upgrade_bot_ss.png", 4,
                                          80};
    return UP_BOT_IMG;
}
const AnimationData& PORTAL_TOP() {
    const static AnimationData PORTAL_TOP{"res/wizards/portal_top.png", 6, 150};
    return PORTAL_TOP;
}
const AnimationData& PORTAL_BOT() {
    const static AnimationData PORTAL_BOT{"res/wizards/portal_bottom.png", 6,
                                          150};
    return PORTAL_BOT;
}

void setDefaults() {
    using WizardSystem::Event;

    ParameterSystem::Params<ROBOT_WIZARD> params;

    params[RobotWizardParams::ShardAmnt]->init(0, Event::ResetT2);

    params[RobotWizardParams::WizCritUpCost]->init(Number(1, 20));

    ParameterSystem::States states;

    states[State::CrysSynBotActive]->init(false);
    states[State::WizSynBotActive]->init(true);
    states[State::TimeWizSynBotActive]->init(true);
}
}  // namespace Constants
}  // namespace RobotWizard
