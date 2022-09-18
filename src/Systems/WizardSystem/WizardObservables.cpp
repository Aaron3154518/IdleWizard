#include "WizardObservables.h"

namespace WizardSystem {
// HideObservable
bool Hidden(WizardId id) { return GetHideObservable()->get(id); }

std::shared_ptr<HideObservable> GetHideObservable() {
    return ServiceSystem::Get<WizardDataService, HideObservable>();
}

// WizardImageObservable
const RenderData& GetWizardImage(WizardId id) {
    return GetWizardImageObservable()->get(id);
}

std::shared_ptr<WizardImageObservable> GetWizardImageObservable() {
    return ServiceSystem::Get<WizardDataService, WizardImageObservable>();
}

// WizardPosObservable
const Rect& GetWizardPos(WizardId id) {
    return GetWizardPosObservable()->get(id);
}

std::shared_ptr<WizardPosObservable> GetWizardPosObservable() {
    return ServiceSystem::Get<WizardDataService, WizardPosObservable>();
}
}  // namespace WizardSystem
