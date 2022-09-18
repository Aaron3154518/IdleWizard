#include "WizardEvents.h"

namespace WizardSystem {
// WizardEventObservable
void WizardEventObservable::next(Event e) {
    switch (e) {
        // Reset tiers
        case Event::ResetT2:
            WizardEventObservableBase::next(Event::ResetT2);
        case Event::ResetT1:
            WizardEventObservableBase::next(Event::ResetT1);
        case Event::NoReset:
            break;
        default:
            WizardEventObservableBase::next(e);
            break;
    }
}

std::shared_ptr<WizardEventObservable> GetWizardEventObservable() {
    return ServiceSystem::Get<WizardEventService, WizardEventObservable>();
}
}  // namespace WizardSystem
