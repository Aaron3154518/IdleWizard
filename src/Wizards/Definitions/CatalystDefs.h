#ifndef CATALYST_DEFS_H
#define CATALYST_DEFS_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem.h>
#include <Wizards/WizardIds.h>

#include <string>
#include <vector>

namespace CatalystDefs {
extern const unsigned int MSPF, NUM_FRAMES;
extern const std::string IMG;

void setDefaults();

RenderDataWPtr GetIcon();
}  // namespace CatalystDefs

#endif
