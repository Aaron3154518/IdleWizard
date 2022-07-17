#include "WizardUpdate.h"

// WizardUpdateObservable
void WizardUpdateObservable::setParam(int key, Number val) {
    mParams[key] = val;
}
const Number& WizardUpdateObservable::getParam(int key, Number defVal) {
    auto it = mParams.find(key);
    return it != mParams.end() ? it->second : defVal;
}

void WizardUpdateObservable::init() {
    mUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            std::bind(&WizardUpdateObservable::onUpdate, this,
                      std::placeholders::_1));
    mUpdateSub->setUnsubscriber(unsub);
}

void WizardUpdateObservable::onUpdate(Time dt) { next(mParams); }