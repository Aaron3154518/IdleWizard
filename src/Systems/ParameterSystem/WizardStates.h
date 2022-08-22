#ifndef WIZARD_STATES_H
#define WIZARD_STATES_H

#include <Systems/ParameterSystem/WizardParameters.h>

namespace State {
enum B : param_t {
    ResetT1 = 0,
};

enum N : param_t {
    BoughtFirstT1 = 0,
    BoughtSecondT1,
    BoughtPowerWizard,
    BoughtTimeWizard,
    BoughtCatalyst,
};
}  // namespace State

enum Events : param_t {
    T1Reset = 0,
};

enum ResetTier : param_t {
    T1 = 0,
};

#endif
