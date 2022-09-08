#ifndef ROBOT_WIZARD_DEFS_H
#define ROBOT_WIZARD_DEFS_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem.h>
#include <Wizards/WizardIds.h>

#include <string>
#include <vector>

namespace RobotWizardDefs {
extern const std::vector<WizardId> TARGETS;

extern const AnimationData IMG, PORTAL_TOP, PORTAL_BOT;

void setDefaults();

RenderDataCWPtr GetIcon();
}  // namespace RobotWizardDefs

#endif
