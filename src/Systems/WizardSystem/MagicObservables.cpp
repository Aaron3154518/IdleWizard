#include "MagicObservables.h"

namespace WizardSystem {
// CatalystMagicObservable
void CatalystMagicObservable::next(const WizardFireball& fb) {
    ParameterSystem::Params<CATALYST> params;
    ParameterSystem::Params<POISON_WIZARD> poiParams;

    Number magic = fb.getPower();

    if (poiParams[PoisonWizardParams::CatGainUp2Lvl].get() > 0) {
        magic ^= poiParams[PoisonWizardParams::CatGainUp2].get();
    } else {
        magic.logTen();
    }

    magic *= poiParams[PoisonWizardParams::CatGainUp1].get();

    next(magic);
}

std::shared_ptr<CatalystMagicObservable> GetCatalystMagicObservable() {
    return ServiceSystem::Get<MagicService, CatalystMagicObservable>();
}
}  // namespace WizardSystem
