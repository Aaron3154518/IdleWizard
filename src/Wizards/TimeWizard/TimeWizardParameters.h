#ifndef TIME_WIZARD_PARAMETERS_H
#define TIME_WIZARD_PARAMETERS_H

#include <Systems/ParameterSystem/ParameterAccess.h>

namespace TimeWizard {
namespace Param {
using ParameterSystem::key_t;

enum P_B : key_t {
    SpeedBaseEffect = 0,
    FreezeBaseEffect,

    FreezeDelay,
    FreezeDuration,

    TimeWarpEffect,

    FreezeUpLvl,
    SpeedUpLvl,
    FBSpeedUpLvl,
    SpeedUpUpLvl,
};

enum P_N : key_t {
    SpeedEffect,
    SpeedCost,
    SpeedUp,
    SpeedUpCost,
    SpeedUpUp,
    SpeedUpCostUp,
    SpeedUpUpCost,

    FreezeEffect,
    FreezeUp,
    FreezeUpCost,
    ClockSpeed,

    FBSpeedUp,
    FBSpeedCost,
};

enum S_B : key_t {
    Active = 0,
    Frozen,
};

enum S_N : key_t {
    TimeWarpEnabled = 0,
};
}  // namespace Param

typedef ParameterSystem::Params<TIME_WIZARD, Param::P_B, Param::P_N, Param::S_B,
                                Param::S_N>
    Params;
}  // namespace TimeWizard

#endif
