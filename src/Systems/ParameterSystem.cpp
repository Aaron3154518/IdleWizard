#include "ParameterSystem.h"

namespace ParameterSystem {
std::shared_ptr<ParameterObservableMap> Get() {
    return ServiceSystem::Get<ParameterService, ParameterObservableMap>();
}

// ParamBase
Number ParamBase::get() const { return 0; }

void ParamBase::set(const Number& val) const {}

ParameterObservable::SubscriptionPtr ParamBase::subscribe(
    std::function<void()> func) const {
    return nullptr;
}

ParameterObservable::SubscriptionPtr ParamListBase::subscribe(
    std::function<void()> func) {
    return nullptr;
}
}  // namespace ParameterSystem
