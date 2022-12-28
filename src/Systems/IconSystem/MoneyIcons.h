#ifndef MONEY_ICONS_H
#define MONEY_ICONS_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/IconSystem/IconSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Wizards/Catalyst/CatalystConstants.h>
#include <Wizards/PoisonWizard/PoisonWizardConstants.h>
#include <Wizards/PowerWizard/PowerWizardConstants.h>
#include <Wizards/Wizard/WizardConstants.h>
#include <Wizards/WizardIds.h>

namespace MoneyIcons {
RenderTextureCPtr GetMoneyIcon(const ParameterSystem::ValueParam& param);
}

#endif
