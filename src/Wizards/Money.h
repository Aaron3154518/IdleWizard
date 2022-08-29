#ifndef MONEY_H
#define MONEY_H

#include <Systems/ParameterSystem/ParameterAccess.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Wizards/WizardIds.h>

#include <string>

namespace Money {
std::string GetMoneyIcon(const ParameterSystem::ValueParam& param);
}

#endif
