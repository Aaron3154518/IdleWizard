#include "WizardData.h"

std::shared_ptr<ParameterMap> Parameters() {
    return ServiceSystem::Get<ParameterService, ParameterMap>();
}
