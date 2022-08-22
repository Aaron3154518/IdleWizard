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
    mResetSub = WizardSystem::GetResetObservable()->subscribe(
        [this](WizardSystem::ResetTier tier) {
            if (mResetTier <= tier) {
                set(mDefault);
            }
        });
}

void BaseValueObservable::init(Number defVal) { init(defVal, mResetTier); }

void BaseValueObservable::init(WizardSystem::ResetTier tier) {
    mResetTier = tier;
}

void BaseValueObservable::init(Number defVal, WizardSystem::ResetTier tier) {
    mDefault = defVal;
    mResetTier = tier;
    set(mDefault);
}

// StateObservable
bool StateObservable::get() const { return mState; }

void StateObservable::set(bool state) {
    mState = state;
    next();
}

// BaseStateObservable
BaseStateObservable::BaseStateObservable() {
    mResetSub = WizardSystem::GetResetObservable()->subscribe(
        [this](WizardSystem::ResetTier tier) {
            if (mResetTier <= tier) {
                set(mDefault);
            }
        });
}

void BaseStateObservable::init(bool defVal) { init(defVal, mResetTier); }

void BaseStateObservable::init(WizardSystem::ResetTier tier) {
    mResetTier = tier;
}

void BaseStateObservable::init(bool defVal, WizardSystem::ResetTier tier) {
    mDefault = defVal;
    mResetTier = tier;
    set(mDefault);
}

}  // namespace ParameterSystem
