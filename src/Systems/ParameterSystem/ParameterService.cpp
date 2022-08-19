#include "ParameterService.h"

namespace ParameterSystem {
const ParameterObservablePtr& ParameterObservableMapImpl::get(WizardId id,
                                                              param_t key) {
    ParameterObservablePtr& ptr = mParams[id][key];
    if (!ptr) {
        ptr = std::make_unique<ParameterObservable>();
    }
    return ptr;
}

std::shared_ptr<ParameterObservableMap> Get() {
    return ServiceSystem::Get<ParameterService, ParameterObservableMap>();
}
}  // namespace ParameterSystem
