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

// WizardImageObservable
const RenderData& GetWizardImage(WizardId id) {
    return GetWizardImageObservable()->get(id);
}

std::shared_ptr<WizardImageObservable> GetWizardImageObservable() {
    return ServiceSystem::Get<WizardService, WizardImageObservable>();
}

// WizardPosObservable
SDL_FPoint GetWizardPos(WizardId id) {
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
