#include "ParameterObservable.h"

namespace ParameterSystem {
const Number& ParameterObservable::get() const { return mVal; }

void ParameterObservable::set(const Number& val) {
    mVal = val;
    ParameterObservableBase::next();
}

void ParameterObservable::onSubscribe(SubscriptionPtr sub) { sub->get<0>()(); }

}  // namespace ParameterSystem
