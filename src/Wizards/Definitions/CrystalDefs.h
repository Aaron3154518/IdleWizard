#ifndef CRYSTAL_DEFS_H
#define CRYSTAL_DEFS_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/WizardSystem.h>
#include <Wizards/WizardIds.h>

#include <string>
#include <vector>

namespace CrystalDefs {
extern const Number T1_COST1, T1_COST2;
extern const SDL_Color MSG_COLOR, GLOW_MSG_COLOR;

extern const std::string WIZ_CNT_UP_IMG, CRYS_GLOW_UP_IMG, FRACTURE_IMG;
extern const AnimationData IMG, GLOW_EFFECT_IMG, GLOW_FINISH_IMG;

void setDefaults();

RenderDataWPtr GetIcon();
}  // namespace CrystalDefs

#endif
