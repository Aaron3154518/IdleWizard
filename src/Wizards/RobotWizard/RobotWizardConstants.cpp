#include "RobotWizardConstants.h"

#include <Components/FireballBase.h>
#include <Wizards/PowerWizard/PowerFireball.h>

namespace RobotWizard {
namespace Constants {
const std::vector<WizardId> UP_TARGETS{CRYSTAL, WIZARD, POWER_WIZARD,
                                       TIME_WIZARD};
const std::unordered_map<WizardId, ParameterSystem::StateParam> SYN_TARGETS = {
    {CRYSTAL, Params::get(Param::CrysSynBotActive)},
    {WIZARD, Params::get(Param::WizSynBotActive)},
    {TIME_WIZARD, Params::get(Param::TimeWizSynBotActive)},
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

    Params params;

    params[Param::ShardAmnt]->init(0, Event::ResetT2);

    params[Param::WizCritUpCost]->init(Number(1, 20));

    params[Param::CrysSynBotActive]->init(false);
    params[Param::WizSynBotActive]->init(true);
    params[Param::TimeWizSynBotActive]->init(true);
}
}  // namespace Constants
}  // namespace RobotWizard