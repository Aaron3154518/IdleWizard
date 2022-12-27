#ifndef ROBOT_WIZARD_DEFS_H
#define ROBOT_WIZARD_DEFS_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Wizards/WizardIds.h>

#include <string>
#include <vector>

// Forward Declaration
class PowerFireball;
class Fireball;

namespace RobotWizardDefs {
extern const std::vector<WizardId> UP_TARGETS;
extern const std::unordered_map<WizardId, ParameterSystem::StateParam>
    SYN_TARGETS;

const AnimationData& IMG();
const AnimationData& UP_BOT_IMG();
const AnimationData& PORTAL_TOP();
const AnimationData& PORTAL_BOT();

bool powerFireballFilter(const PowerFireball& fb, WizardId id);
bool fireballFilter(const Fireball& fb, WizardId id);

void setDefaults();
}  // namespace RobotWizardDefs

#endif
