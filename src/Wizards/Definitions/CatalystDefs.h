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
const AnimationData& IMG();

extern const ParameterSystem::BaseValue REG_FB_CNT, POW_FB_CNT, POI_FB_CNT;
enum FBCntUpBuyType { Reg = 0, Pow, Poi };
extern const std::vector<ParameterSystem::BaseValue> FB_CNT_TYPES;

void setDefaults();
}  // namespace CatalystDefs

#endif
