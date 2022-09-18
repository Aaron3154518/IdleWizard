#ifndef WIZARD_EVENTS_H
#define WIZARD_EVENTS_H

#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/TargetSystem.h>

namespace WizardSystem {
enum Event : uint8_t {
    NoReset = 0,
    ResetT1,
    ResetT2,

    TimeWarp,
};

typedef TargetSystem::TargetObservable<Event> WizardEventObservableBase;
class WizardEventObservable : public WizardEventObservableBase {
   public:
    void next(Event e);
};

std::shared_ptr<WizardEventObservable> GetWizardEventObservable();

class WizardEventService : public Service<WizardEventObservable> {};
}  // namespace WizardSystem

#endif
