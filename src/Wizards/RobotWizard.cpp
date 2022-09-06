#include "RobotWizard.h"

// RobotWizard
RobotWizard::RobotWizard() : WizardBase(POISON_WIZARD) {}

void RobotWizard::init() {
    mImg.set(RobotWizardDefs::IMG).setDest(IMG_RECT);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);

    WizardBase::init();
}
void RobotWizard::setSubscriptions() {
    mAnimTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            [this](Timer& t) {
                mImg.nextFrame();
                WizardSystem::GetWizardImageObservable()->next(mId, mImg);
                return true;
            },
            RobotWizardDefs::IMG);
}
void RobotWizard::setUpgrades() {
    ParameterSystem::Params<POISON_WIZARD> params;
    ParameterSystem::States states;
}
void RobotWizard::setParamTriggers() {
    ParameterSystem::Params<POISON_WIZARD> params;

    mParamSubs.push_back(ParameterSystem::Param(State::BoughtRobotWizard)
                             .subscribe([this](bool bought) {
                                 WizardSystem::GetHideObservable()->next(
                                     mId, !bought);
                             }));
}

void RobotWizard::onRender(SDL_Renderer* r) { WizardBase::onRender(r); }