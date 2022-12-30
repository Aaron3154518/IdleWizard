#ifndef POISON_WIZARD_PARAMETERS_H
#define POISON_WIZARD_PARAMETERS_H

#include <Systems/ParameterSystem/ParameterAccess.h>

namespace PoisonWizard {
namespace Param {
using ParameterSystem::key_t;

enum P_B : key_t {
    BaseSpeed = 0,
    BaseGlobCnt,
    BasePoisonRate,

    PoisonDecay,

    CatPoisonUpCost,
    CatPoisCntUp,

    ShardMultUpLvl,
    PoisonFbUpLvl,
    GlobCntUpLvl,
};

enum P_N : key_t {
    Speed = 0,
    GlobCnt,

    ShardMultUp,
    ShardMultUpCost,
    PoisonFbUp,
    PoisonFbUpCost,
    GlobCntUp,
    GlobCntCost,
};

enum S_B : key_t {
    BoughtCatPois = 0,
};

enum S_N : key_t {};
}  // namespace Param

typedef ParameterSystem::Params<POISON_WIZARD, Param::P_B, Param::P_N,
                                Param::S_B, Param::S_N>
    Params;
}  // namespace PoisonWizard

#endif
