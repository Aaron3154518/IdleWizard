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

bool Hidden(WizardId id) { return GetHideObservable()->isHidden(id); }

std::shared_ptr<HideObservable> GetHideObservable() {
    return ServiceSystem::Get<WizardService, HideObservable>();
}

const RenderData& GetWizardImage(WizardId id) {
    return GetWizardImageObservable()->get(id);
}

std::shared_ptr<WizardImageObservable> GetWizardImageObservable() {
    return ServiceSystem::Get<WizardService, WizardImageObservable>();
}

void Reset(ResetTier tier) { GetResetObservable()->next(tier); }

std::shared_ptr<ResetObservable> GetResetObservable() {
    return ServiceSystem::Get<WizardService, ResetObservable>();
}

SDL_FPoint GetWizardPos(WizardId id) {
    return GetWizardPosObservable()->get(id);
}

std::shared_ptr<WizardPosObservable> GetWizardPosObservable() {
    return ServiceSystem::Get<WizardService, WizardPosObservable>();
}
}  // namespace WizardSystem
