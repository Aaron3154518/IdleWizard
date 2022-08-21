#include "StateObservable.h"

namespace ParameterSystem {
// StateObservable
const bool& StateObservable::get() const { return mVal; }

void StateObservable::set(const bool& val) {
    mVal = val;
    next();
}

void StateObservable::onSubscribe(SubscriptionPtr sub) { sub->get<0>()(); }
}  // namespace ParameterSystem
