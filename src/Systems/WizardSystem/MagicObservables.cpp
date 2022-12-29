#include "MagicObservables.h"

namespace WizardSystem {
// CatalystMagicObservable
void CatalystMagicObservable::next(const Wizard::Fireball& fb) {
    Catalyst::Params params;

    Number magic = fb.getPower();

    if (params[Catalyst::Param::GainUp2Lvl].get() > 0) {
        magic ^= params[Catalyst::Param::GainUp2].get();
    } else {
        magic.logTen();
    }

    magic *= params[Catalyst::Param::GainUp1].get();

    next(magic);
}

std::shared_ptr<CatalystMagicObservable> GetCatalystMagicObservable() {
    return ServiceSystem::Get<MagicService, CatalystMagicObservable>();
}
}  // namespace WizardSystem
