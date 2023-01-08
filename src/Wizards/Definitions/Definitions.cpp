#include "Definitions.h"

namespace Definitions {
const AnimationData& WIZ_IMG(WizardId id) {
    switch (id) {
        case CRYSTAL:
            return Crystal::Constants::IMG();
        case CATALYST:
            return Catalyst::Constants::IMG();
        case WIZARD:
            return Wizard::Constants::IMG();
        case POWER_WIZARD:
            return PowerWizard::Constants::IMG();
        case TIME_WIZARD:
            return TimeWizard::Constants::IMG();
        case POISON_WIZARD:
            return PoisonWizard::Constants::IMG();
        case ROBOT_WIZARD:
            return RobotWizard::Constants::IMG();
        default:
            break;
    }

    const static AnimationData EMPTY;
    return EMPTY;
};
}  // namespace Definitions
