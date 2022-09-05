#include "ParameterObservable.h"

namespace ParameterSystem {
// ValueObservable
const Number& ValueObservable::get() const { return mVal; }

void ValueObservable::set(const Number& val) {
    mVal = val;
    next();
}

// BaseValueObservable
void BaseValueObservable::init(const Number& defVal) {
    mDefault = defVal;
    set(mDefault);
}

void BaseValueObservable::init(WizardSystem::Event event) {
    mResetEvent = event;
    mResetSub = WizardSystem::GetWizardEventObservable()->subscribe(
        [this]() { set(mDefault); }, mResetEvent);
}

void BaseValueObservable::init(const Number& defVal,
                               WizardSystem::Event event) {
    init(defVal);
    init(event);
}

// StateObservable
bool StateObservable::get() const { return mState; }

void StateObservable::set(bool state) {
    mState = state;
    next();
}

// BaseStateObservable
void BaseStateObservable::init(bool defVal) {
    mDefault = defVal;
    set(mDefault);
}

void BaseStateObservable::init(WizardSystem::Event event) {
    mResetEvent = event;
    mResetSub = WizardSystem::GetWizardEventObservable()->subscribe(
        [this]() { set(mDefault); }, mResetEvent);
}

void BaseStateObservable::init(bool defVal, WizardSystem::Event event) {
    init(defVal);
    init(event);
}

}  // namespace ParameterSystem
