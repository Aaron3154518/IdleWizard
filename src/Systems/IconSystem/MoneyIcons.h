#ifndef MONEY_ICONS_H
#define MONEY_ICONS_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/UpdateServices/TimerService.h>
#include <Systems/IconSystem/IconSystem.h>
#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Wizards/Catalyst/CatalystConstants.h>
#include <Wizards/Catalyst/CatalystParameters.h>
#include <Wizards/Crystal/CrystalParameters.h>
#include <Wizards/PoisonWizard/PoisonWizardConstants.h>
#include <Wizards/PowerWizard/PowerWizardConstants.h>
#include <Wizards/RobotWizard/RobotWizardParameters.h>
#include <Wizards/Wizard/WizardConstants.h>
#include <Wizards/WizardIds.h>

namespace MoneyIcons {
RenderTextureCPtr Get(const ParameterSystem::ValueParam& param);
}

#endif
