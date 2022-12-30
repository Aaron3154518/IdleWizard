#ifndef CRYSTAL_PARAMETERS_H
#define CRYSTAL_PARAMETERS_H

#include <Systems/ParameterSystem/ParameterAccess.h>

namespace Crystal {
namespace Param {
using ParameterSystem::key_t;

enum P_B : key_t {
    Magic = 0,
    Shards,
    BestMagic,
    PoisonMagic,
    PoisonRate,

    WizardCntUpCost,
    GlowUpCost,
    CatalystCost,
    PoisonWizCost,
    RobotCost,
    T1ResetCost,
};

enum P_N : key_t {
    MagicEffect = 0,
    ShardGain,

    NumWizards,
    WizardCntEffect,
    GlowUp,
    GlowEffect,

    T1CostMult,
    T1WizardCost,
};

enum S_B : key_t {
    ResetT1 = 0,

    BoughtPowerWizard,
    BoughtTimeWizard,
    BoughtCatalyst,
    BoughtPoisonWizard,
    BoughtRobotWizard,

    BoughtCrysWizCntUp,
    BoughtCrysGlowUp,

    CrysGlowActive,
};

enum S_N : key_t {
    BoughtFirstT1 = 0,
    BoughtSecondT1,
};
}  // namespace Param

typedef ParameterSystem::Params<CRYSTAL, Param::P_B, Param::P_N, Param::S_B,
                                Param::S_N>
    Params;
}  // namespace Crystal

#endif
