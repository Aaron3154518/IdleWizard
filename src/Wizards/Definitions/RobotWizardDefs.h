#ifndef ROBOT_WIZARD_DEFS_H
#define ROBOT_WIZARD_DEFS_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Wizards/WizardIds.h>

#include <string>
#include <vector>

namespace RobotWizardDefs {
extern const std::vector<WizardId> TARGETS;

const AnimationData& IMG();
const AnimationData& PORTAL_TOP();
const AnimationData& PORTAL_BOT();

void setDefaults();
}  // namespace RobotWizardDefs

#endif
