#ifndef CATALYST_PARAMETERS_H
#define CATALYST_PARAMETERS_H

#include <Systems/ParameterSystem/ParameterAccess.h>

namespace Catalyst {
namespace Param {
using ParameterSystem::key_t;

enum P_B : key_t {
    Magic = 0,

    FBRegCnt,
    FBPowCnt,
    FBPoiCnt,
    RingPoisCnt,

    BaseRange,

    RangeUpLvl,
    ZapCntUpLvl,
    ZapperUpLvl,
    GainUp1Lvl,
    GainUp2Lvl,
    CapUpLvl,
    FBCntLvl,

    ShardGainUpCost,
    MultUpCost,
};

enum P_N : key_t {
    MagicEffect = 0,
    Capacity,

    FBCntEffect,
    FBCntMaxLvl,
    FBCntUpCost,

    Range,
    RangeUpCost,
    RangeUp,

    ZapCntUp,
    ZapCntUpCost,
    ZapperUp,
    ZapperUpCost,

    GainUp1,
    GainUp1Cost,
    GainUp2,
    GainUp2Cost,
    CapUp,
    CapUpCost,

    ShardGainUp,
    CatMultUp,
    ShardMultUp,
};

enum S_B : key_t {
    BoughtShardMult = 0,
    BoughtMultUp,
};

enum S_N : key_t {};
}  // namespace Param

typedef ParameterSystem::Params<CATALYST, Param::P_B, Param::P_N, Param::S_B,
                                Param::S_N>
    Params;
}  // namespace Catalyst

#endif
