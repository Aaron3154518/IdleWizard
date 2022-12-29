#ifndef WIZARD_DEFS_H
#define WIZARD_DEFS_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Wizards/Wizard/WizardParameters.h>
#include <Wizards/WizardIds.h>

#include <string>
#include <vector>

namespace Wizard {
namespace Constants {
const AnimationData& IMG();
const AnimationData& POWER_BKGRND();
const AnimationData& FB_IMG();
const AnimationData& FB_BUFFED_IMG();
const AnimationData& FB_POISON_IMG();
const AnimationData& FB_INNER_IMG();
const AnimationData& FB_OUTER_IMG();
const AnimationData& FB_OUTER_BUFFED_IMG();
const AnimationData& FB_INNER_POISON_IMG();

extern const std::string POWER_UP_IMG, SPEED_UP_IMG, MULTI_UP_IMG, CRIT_UP_IMG;

extern const std::vector<WizardId> TARGETS;

void setDefaults();
}  // namespace Constants
}  // namespace Wizard

#endif
