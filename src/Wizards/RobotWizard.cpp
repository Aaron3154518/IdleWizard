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
    mMoveUpdateSub = TimeSystem::GetUpdateObservable()->subscribe(
        [this](Time dt) { onUpdate(dt); });
    // mUpTimerSub = TimeSystem::GetTimerObservable()->subscribe(
    //[this](Timer& t) { return onUpTimer(t); }, Timer(2000));
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

void RobotWizard::onUpdate(Time dt) {
    Rect pos =
        WizardSystem::GetWizardPos(RobotWizardDefs::TARGETS.at(mTargetIdx));
    float dx = pos.cX() - mPos->rect.cX(), dy = pos.cY() - mPos->rect.cY();
    float mag = sqrtf(dx * dx + dy * dy);

    if (mag <= 10) {
        Timer t;
        onUpTimer(t);
    } else {
        float frac = 100 * dt.s() / mag;
        setPos(mPos->rect.cX() + dx * frac, mPos->rect.cY() + dy * frac);
    }
}
void RobotWizard::onRender(SDL_Renderer* r) { WizardBase::onRender(r); }
bool RobotWizard::onUpTimer(Timer& t) {
    WizardId target = RobotWizardDefs::TARGETS.at(mTargetIdx);
    auto catMagic = ParameterSystem::Param<CATALYST>(CatalystParams::Magic);
    Number maxSpend = catMagic.get() / 10 + 1;
    Number spent = GetWizardUpgrades(target)->buyAll(
        UpgradeDefaults::CRYSTAL_MAGIC, catMagic.get() / 10);
    catMagic.set(catMagic.get() - spent);
    do {
        mTargetIdx = (mTargetIdx + 1) % RobotWizardDefs::TARGETS.size();
    } while (RobotWizardDefs::TARGETS.at(mTargetIdx) != CRYSTAL &&
             WizardSystem::Hidden(RobotWizardDefs::TARGETS.at(mTargetIdx)));
    return true;
}
