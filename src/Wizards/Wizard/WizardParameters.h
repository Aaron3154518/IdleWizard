#ifndef WIZARD_PARAMETERS_H
#define WIZARD_PARAMETERS_H

#include <Systems/ParameterSystem/ParameterAccess.h>

namespace Wizard {
namespace Param {
using ParameterSystem::key_t;

enum P_B : key_t {
    BasePower = 0,
    BaseCrit,
    BaseCritSpread,
    BaseSpeed,
    BaseFBSpeed,

    PowerWizEffect,

    PowerUpLvl,
    MultiUpLvl,
    CritUpLvl,
    RoboCritUpLvl,
};

enum P_N : key_t {
    Power = 0,
    PowerUp,
    PowerUpCost,
    PowerUpMaxLvl,

    Crit,
    CritUp,
    CritSpread,
    CritSpreadUp,
    CritUpCost,
    RoboCritUp,
    RoboCritUpCost,

    Speed,
    FBSpeed,
    FBSpeedEffect,

    MultiUp,
    MultiUpCost,
};

enum S_B : key_t {
    Boosted = 0,
};

enum S_N : key_t {};
}  // namespace Param

typedef ParameterSystem::Params<WIZARD, Param::P_B, Param::P_N, Param::S_B,
                                Param::S_N>
    Params;
}  // namespace Wizard

#endif
