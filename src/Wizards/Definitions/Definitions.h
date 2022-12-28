#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#include <RenderSystem/RenderTypes.h>
#include <Wizards/Catalyst/CatalystConstants.h>
#include <Wizards/Crystal/CrystalConstants.h>
#include <Wizards/PoisonWizard/PoisonWizardConstants.h>
#include <Wizards/PowerWizard/PowerWizardConstants.h>
#include <Wizards/RobotWizard/RobotWizardConstants.h>
#include <Wizards/TimeWizard/TimeWizardConstants.h>
#include <Wizards/Wizard/WizardConstants.h>
#include <Wizards/WizardIds.h>

#include <unordered_map>

namespace Definitions {
const AnimationData& WIZ_IMG(WizardId id);
}

#endif
