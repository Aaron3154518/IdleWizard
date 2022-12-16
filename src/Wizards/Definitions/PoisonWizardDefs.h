#ifndef POISON_WIZARD_DEFS_H
#define POISON_WIZARD_DEFS_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Wizards/WizardIds.h>

#include <string>
#include <vector>

namespace PoisonWizardDefs {
const AnimationData& IMG();
const AnimationData& GLOB_IMG();
const AnimationData& BUBBLE1_IMG();
const AnimationData& BUBBLE2_IMG();

void setDefaults();
}  // namespace PoisonWizardDefs

#endif
