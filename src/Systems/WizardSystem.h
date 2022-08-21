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
enum Event {
    // States
    BoughtFirstT1 = 0,
    BoughtSecondT1,
    BoughtPowerWizard,
    BoughtTimeWizard,
    BoughtCatalyst,

    // Triggers
    ResetT1
};

typedef ParameterSystem::StateObservableMap<Event> EventObservableMap;

class WizardService : public Service<HideObservable, EventObservableMap> {};

typedef ParameterSystem::StateAccess<WizardService, Event> Events;
}  // namespace WizardSystem

#endif
