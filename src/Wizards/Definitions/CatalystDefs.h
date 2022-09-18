#ifndef CATALYST_DEFS_H
#define CATALYST_DEFS_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Wizards/WizardIds.h>

#include <string>
#include <vector>

namespace CatalystDefs {
extern const unsigned int MSPF, NUM_FRAMES;
extern const AnimationData IMG;

void setDefaults();
}  // namespace CatalystDefs

#endif
