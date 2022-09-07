#ifndef POISON_WIZARD_DEFS_H
#define POISON_WIZARD_DEFS_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem.h>
#include <Wizards/WizardIds.h>

#include <string>
#include <vector>

namespace PoisonWizardDefs {
extern const AnimationData IMG;

void setDefaults();

RenderDataCWPtr GetIcon();
}  // namespace PoisonWizardDefs

#endif