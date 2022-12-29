#ifndef POWER_WIZARD_PARAMETERS_H
#define POWER_WIZARD_PARAMETERS_H

#include <Systems/ParameterSystem/ParameterAccess.h>

namespace PowerWizard {
namespace Param {
using ParameterSystem::key_t;

enum P_B : key_t {
    BasePower = 0,
    BaseSpeed,
    BaseFBSpeed,

    PowerUpLvl,
    TimeWarpUpLvl,
};

enum P_N : key_t {
    Duration = 0,

    Power,
    PowerUp,
    PowerUpCost,
    FireRingEffect,

    Speed,
    FBSpeed,
    FBSpeedEffect,

    TimeWarpUp,
    TimeWarpUpCost,
};

enum S_B : key_t {};

enum S_N : key_t {};
}  // namespace Param

typedef ParameterSystem::Params<POWER_WIZARD, Param::P_B, Param::P_N,
                                Param::S_B, Param::S_N>
    Params;
}  // namespace PowerWizard

#endif
