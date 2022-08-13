#include "WizardData.h"

#include "Wizard.h"

void WizardsDataObservable::set(WizardBase* base) {
    switch (base->mId) {
        case WizardId::WIZARD: {
            Wizard* wizard = (Wizard*)base;
            if (wizard) {
                mData.wizard = wizard->getData();
            }
        } break;
        case WizardId::CRYSTAL: {
            Crystal* crystal = (Crystal*)base;
            if (crystal) {
                mData.crystal = crystal->getData();
            }
        } break;
        case WizardId::CATALYST: {
            Catalyst* catalyst = (Catalyst*)base;
            if (catalyst) {
                mData.catalyst = catalyst->getData();
            }
        } break;
    }
}

void WizardsDataObservable::init() {
    mUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            std::bind(&WizardsDataObservable::onUpdate, this,
                      std::placeholders::_1));
}

void WizardsDataObservable::onUpdate(Time dt) { next(mData, dt); }
