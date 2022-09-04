#include "WizardSystem.h"

namespace WizardSystem {
// HideObservable
bool Hidden(WizardId id) { return GetHideObservable()->get(id); }

std::shared_ptr<HideObservable> GetHideObservable() {
    return ServiceSystem::Get<WizardService, HideObservable>();
}

// WizardImageObservable
const RenderData& GetWizardImage(WizardId id) {
    return GetWizardImageObservable()->get(id);
}

std::shared_ptr<WizardImageObservable> GetWizardImageObservable() {
    return ServiceSystem::Get<WizardService, WizardImageObservable>();
}

// WizardPosObservable
const Rect& GetWizardPos(WizardId id) {
    return GetWizardPosObservable()->get(id);
}

std::shared_ptr<WizardPosObservable> GetWizardPosObservable() {
    return ServiceSystem::Get<WizardService, WizardPosObservable>();
}

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
    return ServiceSystem::Get<WizardService, WizardEventObservable>();
}

}  // namespace WizardSystem
