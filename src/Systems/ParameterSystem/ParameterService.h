#ifndef PARAMETER_SYSTEM_H
#define PARAMETER_SYSTEM_H

#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/ParameterSystem/ParameterObservable.h>
#include <Systems/ParameterSystem/WizardParams.h>
#include <Utils/Number.h>
#include <Wizards/WizardIds.h>

#include <initializer_list>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

namespace ParameterSystem {
typedef std::unique_ptr<ParameterObservable> ParameterObservablePtr;
typedef std::unordered_map<param_t, ParameterObservablePtr>
    ParameterObservableList;

// Stores numbers by wizard id and param enum
class ParameterObservableMapImpl {
   protected:
    const ParameterObservablePtr& get(WizardId id, param_t key);

   private:
    std::unordered_map<WizardId, ParameterObservableList> mParams;
};

class ParameterObservableMap : public ParameterObservableMapImpl,
                               public ObservableBase {
    friend class ParamBase;
    friend class ParamListBase;
    template <WizardId id>
    friend class ParamList;
    friend class ParamMapBase;
};

class ParameterService : public Service<ParameterObservableMap> {};

std::shared_ptr<ParameterObservableMap> Get();
}  // namespace ParameterSystem

#endif