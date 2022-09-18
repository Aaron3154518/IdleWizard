#include "MagicObservables.h"

namespace WizardSystem {
// CrystalMagicObservable
std::shared_ptr<CrystalMagicObservable> GetCrystalMagicObservable() {
    return ServiceSystem::Get<MagicService, CrystalMagicObservable>();
}

// CatalystMagicObservable
std::shared_ptr<CatalystMagicObservable> GetCatalystMagicObservable() {
    return ServiceSystem::Get<MagicService, CatalystMagicObservable>();
}
}  // namespace WizardSystem
