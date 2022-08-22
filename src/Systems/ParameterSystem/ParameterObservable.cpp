#include "ParameterObservable.h"

namespace ParameterSystem {
// ValueObservable
const Number& ValueObservable::get() const { return mVal; }

void ValueObservable::set(const Number& val) {
    mVal = val;
    next();
}

// BaseValueObservable
BaseValueObservable::BaseValueObservable() {
    set(mDefault);
    setResetTier(mResetTier);
}

void BaseValueObservable::setResetTier(ResetTier tier) {
    mResetTier = tier;
    // mResetSub = TBD
}

// StateObservable
bool StateObservable::get() const { return mState; }

void StateObservable::set(bool state) {
    mState = state;
    next();
}

// BaseStateObservable
BaseStateObservable::BaseStateObservable() {
    set(mDefault);
    setResetTier(mResetTier);
}

void BaseStateObservable::setResetTier(ResetTier tier) {
    mResetTier = tier;
    // mResetSub = TBD
}
}  // namespace ParameterSystem
