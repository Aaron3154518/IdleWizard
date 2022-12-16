#include "Definitions.h"

namespace Definitions {
const AnimationData& WIZ_IMG(WizardId id) {
    switch (id) {
        case CRYSTAL:
            return CrystalDefs::IMG();
        case CATALYST:
            return CatalystDefs::IMG();
        case WIZARD:
            return WizardDefs::IMG();
        case POWER_WIZARD:
            return PowerWizardDefs::IMG();
        case TIME_WIZARD:
            return TimeWizardDefs::IMG();
        case POISON_WIZARD:
            return PoisonWizardDefs::IMG();
        case ROBOT_WIZARD:
            return RobotWizardDefs::IMG();
    }
};
}  // namespace Definitions
