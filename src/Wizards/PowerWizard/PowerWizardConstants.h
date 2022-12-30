#ifndef POWER_WIZARD_DEFS_H
#define POWER_WIZARD_DEFS_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Wizards/PowerWizard/PowerWizardParameters.h>
#include <Wizards/WizardIds.h>

#include <string>
#include <vector>

namespace PowerWizard {
namespace Constants {
const AnimationData& IMG();
const AnimationData& FB_IMG();
extern const std::string POWER_UP_IMG, TIME_WARP_UP_IMG;

void setDefaults();
}  // namespace Constants
}  // namespace PowerWizard

#endif
