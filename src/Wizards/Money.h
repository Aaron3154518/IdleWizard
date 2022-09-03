#ifndef MONEY_H
#define MONEY_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Wizards/WizardIds.h>

namespace Money {
RenderDataWPtr GetMoneyIcon(const ParameterSystem::ValueParam& param);
}

#endif
