#ifndef WIZARD_STATES_H
#define WIZARD_STATES_H

#include <Systems/ParameterSystem/WizardParameters.h>

namespace State {
enum B : param_t {
    ResetT1 = 0,

    BoughtCrysWizCntUp,
    BoughtCrysGlowUp,

    BoughtCatShardMult,

    BoughtPowerWizard,
    BoughtTimeWizard,
    BoughtCatalyst,
    BoughtPoisonWizard,
    BoughtRobotWizard,

    WizBoosted,

    CrysGlowActive,
    CrysPoisoned,

    TimeWizActive,
    TimeWizFrozen,
};

enum N : param_t {
    BoughtFirstT1 = 0,
    BoughtSecondT1,

    TimeWarpEnabled,

    ShootRobot,
};
}  // namespace State

#endif
