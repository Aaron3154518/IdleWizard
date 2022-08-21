#ifndef WIZARD_SYSTEM_H
#define WIZARD_SYSTEM_H

#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/ParameterSystem/StateService.h>
#include <Wizards/WizardIds.h>

namespace WizardSystem {
// For handling wizard hide/show events
typedef ForwardObservable<void(WizardId, bool)> HideObservableBase;
class HideObservable : public HideObservableBase {
   public:
    void next(WizardId id, bool hide);

    bool isHidden(WizardId id) const;

   private:
    std::unordered_map<WizardId, bool> mHidden;
};

bool Hidden(WizardId id);

std::shared_ptr<HideObservable> GetHideObservable();

// For handling general wizard events
namespace State {
enum _ : uint8_t {
    BoughtFirstT1 = 0,
    BoughtSecondT1,
    BoughtPowerWizard,
    BoughtTimeWizard,
    BoughtCatalyst,
    ResetT1
};
}

namespace Event {
enum _ : uint8_t {
    T1Reset = 0,
};
}

typedef ParameterSystem::StateObservableMap<State::_> StateObservableMap;
typedef ParameterSystem::StateObservableMap<Event::_> EventObservableMap;

class WizardService
    : public Service<HideObservable, StateObservableMap, EventObservableMap> {};

typedef ParameterSystem::StateAccess<WizardService, State::_> States;
struct Events : public ParameterSystem::StateAccess<WizardService, Event::_> {
    static void send(Event::_ e);
};
}  // namespace WizardSystem

#endif
