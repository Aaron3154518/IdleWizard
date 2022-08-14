#include "WizardData.h"

std::shared_ptr<ParameterMap> Parameters() {
    return ServiceSystem::Get<ParameterService, ParameterMap>();
}

// ParamBase
Number ParamBase::get() const { return 0; }

void ParamBase::set(const Number& val) const {}

ParameterObservable::SubscriptionPtr ParamBase::subscribe(
    std::function<void()> func) const {
    return nullptr;
}
