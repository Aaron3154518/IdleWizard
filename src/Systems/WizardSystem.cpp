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

// Events
void Events::send(Event::_ e) { set(e, true); }

}  // namespace WizardSystem
