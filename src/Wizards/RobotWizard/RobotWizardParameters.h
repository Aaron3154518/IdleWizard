#ifndef ROBOT_WIZARD_PARAMETERS_H
#define ROBOT_WIZARD_PARAMETERS_H

#include <Systems/ParameterSystem/ParameterAccess.h>

namespace RobotWizard {
namespace Param {
using ParameterSystem::key_t;

enum P_B : key_t {
    Shards = 0,
    UpBotBaseCap,
    UpBotBaseRate,

    ShardPowerUpCost,
    WizCritUpCost,
    UpBotCapRateUpCost,
    NewCatUpsCost,

    UpBotCost,
    WizSynBotCost,
    CrysSynBotCost,
    TimeWizSynBotCost,
};

enum P_N : key_t {
    ShardPowerUp = 0,

    UpBotCap,
    UpBotCapUp,
    UpBotRate,
    UpBotRateUp,
};

enum S_B : key_t {
    BoughtShardPowerUp = 0,
    BoughtWizCritUp,
    BoughtUpBotCapRateUp,
    BoughtNewCatUps,

    UpBotActive,
    WizSynBotActive,
    CrysSynBotActive,
    TimeWizSynBotActive,
};

enum S_N : key_t {};
}  // namespace Param

typedef ParameterSystem::Params<ROBOT_WIZARD, Param::P_B, Param::P_N,
                                Param::S_B, Param::S_N>
    Params;
}  // namespace RobotWizard

#endif
