#ifndef ROBOT_WIZARD_PARAMETERS_H
#define ROBOT_WIZARD_PARAMETERS_H

#include <Systems/ParameterSystem/ParameterAccess.h>

namespace RobotWizard {
namespace Param {
using ParameterSystem::key_t;

enum P_B : key_t {
    ShardAmnt = 0,

    U1,
    U2,
    U3,
    U4,
    U5,
    U6,
    U7,
    U8,
    U9,
    U10,

    WizCritUpCost,
};

enum P_N : key_t {};

enum S_B : key_t {
    BoughtRoboWizCritUp = 0,

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
