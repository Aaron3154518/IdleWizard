#include "FireballService.h"

// FireballObservable
void FireballObservable::onSubscribe(SubscriptionPtr sub) {
    sub->get<FUNC>()(mTargets[sub->get<DATA>()]);
}

void FireballObservable::next(WizardId id, SDL_FPoint pos) {
    if (id == WizardId::size) {
        throw std::runtime_error("Cannot use size as a target");
    }
    mTargets[id] = pos;
    for (auto sub : *this) {
        if (sub->get<DATA>() == id) {
            sub->get<FUNC>()(pos);
        }
    }
}

SDL_FPoint FireballObservable::getPos(WizardId id) const {
    if (id == WizardId::size) {
        throw std::runtime_error("Cannot use size as a target");
    }
    return mTargets[id];
}
