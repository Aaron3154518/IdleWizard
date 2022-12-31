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
const AnimationData& BOT_IMG() {
    const static AnimationData UP_BOT_IMG{"res/wizards/upgrade_bot_ss.png", 4,
                                          80};
    return UP_BOT_IMG;
}
const std::string& BOT_HAT_IMG(WizardId id) {
    const static std::string WIZ_HAT = "res/wizards/bot_wiz_hat.png",
                             CRYS_HAT = "res/wizards/bot_crys_hat.png",
                             TIME_WIZ_HAT = "res/wizards/bot_time_wiz_hat.png";

    switch (id) {
        case WIZARD:
            return WIZ_HAT;
        case CRYSTAL:
            return CRYS_HAT;
        case TIME_WIZARD:
            return TIME_WIZ_HAT;
    }

    throw std::runtime_error("SYN_BOT_IMG(): Invalid Wizard Id (" +
                             std::to_string(id) + ")");

    const static std::string EMPTY;
    return EMPTY;
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

    params[Param::CrysSynBotActive]->init(true);
    params[Param::WizSynBotActive]->init(true);
    params[Param::TimeWizSynBotActive]->init(true);
}
}  // namespace Constants
}  // namespace RobotWizard
