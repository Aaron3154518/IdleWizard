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
    mUpTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) { return onUpTimer(t); }, Timer(2000));
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
bool RobotWizard::onUpTimer(Timer& t) {
    WizardId target = RobotWizardDefs::TARGETS.at(mTargetIdx);
    auto catMagic = ParameterSystem::Param<CATALYST>(CatalystParams::Magic);
    auto cryMagic = ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic);
    Number maxSpend = catMagic.get() / 10 + 1;
    Number magic = cryMagic.get();
    cryMagic.set(maxSpend);
    Number spent = GetWizardUpgrades(target)->buyAll(
        UpgradeDefaults::CRYSTAL_MAGIC, catMagic.get() / 10);
    catMagic.set(catMagic.get() - spent);
    cryMagic.set(magic);
    mTargetIdx = (mTargetIdx + 1) % RobotWizardDefs::TARGETS.size();
    return true;
}
