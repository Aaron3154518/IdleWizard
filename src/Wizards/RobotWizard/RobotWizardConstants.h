#ifndef ROBOT_WIZARD_DEFS_H
#define ROBOT_WIZARD_DEFS_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Wizards/RobotWizard/RobotWizardParameters.h>
#include <Wizards/WizardIds.h>

#include <random>
#include <string>
#include <vector>

namespace RobotWizard {
namespace Constants {
extern const std::vector<WizardId> UP_TARGETS;
extern const std::unordered_map<WizardId, ParameterSystem::StateParam>
    SYN_TARGETS;

const AnimationData& IMG();
const AnimationData& BOT_IMG();
const std::string& BOT_HAT_IMG(WizardId id);
RenderTextureCPtr BOT_FLOAT_IMG(WizardId id);
const AnimationData& PORTAL_TOP();
const AnimationData& PORTAL_BOT();

void setDefaults();
}  // namespace Constants
}  // namespace RobotWizard

#endif
