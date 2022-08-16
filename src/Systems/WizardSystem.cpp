#include "WizardSystem.h"

namespace WizardSystem {
// HideObservable
void HideObservable::next(WizardId id, bool hide) {
    HideObservableBase::next(id, hide);
    mHidden[id] = hide;
}

bool HideObservable::isHidden(WizardId id) const {
    auto it = mHidden.find(id);
    return it != mHidden.end() && it->second;
}

bool Hidden(WizardId id) {
    return ServiceSystem::Get<WizardService, HideObservable>()->isHidden(id);
}

std::shared_ptr<HideObservable> GetHideObservable() {
    return ServiceSystem::Get<WizardService, HideObservable>();
}

std::shared_ptr<WizEventsObservable> GetWizEventsObservable() {
    return ServiceSystem::Get<WizardService, WizEventsObservable>();
}

void FireWizEvent(Event e) { return GetWizEventsObservable()->next(e); }

}  // namespace WizardSystem
