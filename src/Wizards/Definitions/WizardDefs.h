#ifndef WIZARD_DEFS_H
#define WIZARD_DEFS_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem.h>
#include <Wizards/WizardIds.h>

#include <string>
#include <vector>

namespace WizardDefs {
extern const AnimationData IMG, POWER_BKGRND, FB_IMG, FB_INNER_IMG,
    FB_OUTER_IMG, FB_OUTER_BUFFED_IMG, FB_INNER_POISON_IMG;

extern const std::string POWER_UP_IMG, SPEED_UP_IMG, MULTI_UP_IMG, CRIT_UP_IMG;

extern const std::vector<WizardId> TARGETS;

void setDefaults();
}  // namespace WizardDefs

#endif
