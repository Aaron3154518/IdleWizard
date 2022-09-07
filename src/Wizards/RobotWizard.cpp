#include "RobotWizard.h"

// RobotWizard
RobotWizard::RobotWizard() : WizardBase(ROBOT_WIZARD) {}

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
    mPowFireballHitSub = PowerWizFireball::GetHitObservable()->subscribe(
        [this](const PowerWizFireball& f) { onPowFireballHit(f); }, mId);
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
    ParameterSystem::Params<ROBOT_WIZARD> params;
    ParameterSystem::States states;

    mParamSubs.push_back(
        states[State::BoughtRobotWizard].subscribe([this](bool bought) {
            WizardSystem::GetHideObservable()->next(mId, !bought);
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
void RobotWizard::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    TextureBuilder tex;

    RenderDataCPtr ptr = PowerWizFireball::GetIcon().lock();
    if (ptr) {
        float w = IMG_RECT.minDim() / 3;
        Rect imgR(0, 0, w, w);
        RenderData img = RenderData(*ptr);
        for (auto pair : mPowFireballs) {
            switch (pair.first) {
                case WIZARD:
                    imgR.setPos(mPos->rect.x(), mPos->rect.y2(),
                                Rect::Align::CENTER);
                    break;
                case CRYSTAL:
                    imgR.setPos(mPos->rect.cX(), mPos->rect.y2(),
                                Rect::Align::CENTER, Rect::Align::TOP_LEFT);
                    break;
                case TIME_WIZARD:
                    imgR.setPos(mPos->rect.x2(), mPos->rect.y2(),
                                Rect::Align::CENTER);
                    break;
            }
            tex.draw(img.setDest(imgR));
        }
    }
}
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
void RobotWizard::onPowFireballHit(const PowerWizFireball& fireball) {
    WizardId target = fireball.getTargetId();

    auto it = mPowFireballs.find(target);
    if (it == mPowFireballs.end() || fireball.getPower() > it->second.power ||
        fireball.getDuration() > it->second.duration) {
        mPowFireballs[target] = fireball.getData();
    }
}
