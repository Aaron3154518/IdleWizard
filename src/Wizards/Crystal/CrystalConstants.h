#ifndef CRYSTAL_DEFS_H
#define CRYSTAL_DEFS_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Wizards/Crystal/CrystalParameters.h>
#include <Wizards/WizardIds.h>

#include <string>
#include <vector>

namespace Crystal {
namespace Constants {
extern const Number T1_COST1, T1_COST2;
extern const SDL_Color MSG_COLOR, GLOW_MSG_COLOR, POISON_MSG_COLOR;

extern const std::string WIZ_CNT_UP_IMG, CRYS_GLOW_UP_IMG, FRACTURE_IMG;
const AnimationData& IMG();
const AnimationData& GLOW_EFFECT_IMG();
const AnimationData& GLOW_FINISH_IMG();

void setDefaults();
}  // namespace Constants
}  // namespace Crystal

#endif
