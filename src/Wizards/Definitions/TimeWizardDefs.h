#ifndef TIME_WIZARD_DEFS_H
#define TIME_WIZARD_DEFS_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem.h>
#include <Wizards/WizardIds.h>

#include <string>
#include <vector>

namespace TimeWizardDefs {
extern const AnimationData IMG, FREEZE_IMG;

extern const std::string FREEZE_UP_IMG, SPEED_UP_IMG, FB_SPEED_UP_IMG,
    POW_SPEED_UP_IMG, ACTIVE_UP_IMG;

void setDefaults();

RenderDataCWPtr GetIcon();
}  // namespace TimeWizardDefs

#endif
