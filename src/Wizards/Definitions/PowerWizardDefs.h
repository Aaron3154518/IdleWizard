#ifndef POWER_WIZARD_DEFS_H
#define POWER_WIZARD_DEFS_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem.h>
#include <Wizards/WizardIds.h>

#include <string>
#include <vector>

namespace PowerWizardDefs {
extern const AnimationData IMG;
extern const std::string POWER_UP_IMG;

void setDefaults();

RenderDataWPtr GetIcon();
}  // namespace PowerWizardDefs

#endif
