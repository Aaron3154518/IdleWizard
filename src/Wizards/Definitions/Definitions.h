#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <RenderSystem/RenderTypes.h>
#include <Wizards/Definitions/CatalystDefs.h>
#include <Wizards/Definitions/CrystalDefs.h>
#include <Wizards/Definitions/PoisonWizardDefs.h>
#include <Wizards/Definitions/PowerWizardDefs.h>
#include <Wizards/Definitions/RobotWizardDefs.h>
#include <Wizards/Definitions/TimeWizardDefs.h>
#include <Wizards/Definitions/WizardDefs.h>
#include <Wizards/WizardIds.h>

#include <unordered_map>

namespace Definitions {
const AnimationData& WIZ_IMG(WizardId id);
}

#endif
