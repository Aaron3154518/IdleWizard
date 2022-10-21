#include "MagicObservables.h"

namespace WizardSystem {
// CatalystMagicObservable
void CatalystMagicObservable::next(const WizardFireball& fb) {
    ParameterSystem::Params<CATALYST> params;

    Number magic = fb.getPower();

    if (params[CatalystParams::GainUp2Lvl].get() > 0) {
        magic ^= params[CatalystParams::GainUp2].get();
    } else {
        magic.logTen();
    }

    magic *= params[CatalystParams::GainUp1].get();

    next(magic);
}

std::shared_ptr<CatalystMagicObservable> GetCatalystMagicObservable() {
    return ServiceSystem::Get<MagicService, CatalystMagicObservable>();
}
}  // namespace WizardSystem
