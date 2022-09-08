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
    mTpUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            [this](Time dt) { onTpUpdate(dt); });
    mMoveUpdateSub = TimeSystem::GetUpdateObservable()->subscribe(
        [this](Time dt) { onMoveUpdate(dt); });
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

    mParamSubs.push_back(
        states[State::TimeWizFrozen].subscribe([this](bool frozen) {
            if (frozen) {
                mTpQueue.push(WIZARD);
            } else {
                mTpQueue.push(CRYSTAL);
                mTpQueue.push(TIME_WIZARD);
            }
        }));
}

void RobotWizard::onTpUpdate(Time dt) {
    if (!mTpQueue.empty()) {
        auto it = mStoredFireballs.find(mTpQueue.front());
        while (it == mStoredFireballs.end()) {
            mTpQueue.pop();
            if (mTpQueue.empty()) {
                break;
            }
            it = mStoredFireballs.find(mTpQueue.front());
        }
        if (!mTpQueue.empty()) {
            Rect pos = WizardSystem::GetWizardPos(it->first);
            setPos(pos.cX() + (mPos->rect.w() * (rDist(gen) * .5 + .25)),
                   pos.cY() + (mPos->rect.h() * (rDist(gen) * .5 + .25)));
            mFireballs.push_back(ComponentFactory<PowerWizFireball>::New(
                SDL_FPoint{mPos->rect.cX(), mPos->rect.cY()}, it->first,
                it->second));
            mTpQueue.pop();
            mStoredFireballs.erase(it);
        }
    }

    for (auto it = mFireballs.begin(); it != mFireballs.end(); ++it) {
        if ((*it)->dead()) {
            it = mFireballs.erase(it);
            if (it == mFireballs.end()) {
                break;
            }
        }
    }
}
void RobotWizard::onMoveUpdate(Time dt) {
    if (mTpQueue.empty()) {
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
}
void RobotWizard::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    TextureBuilder tex;

    RenderDataCPtr ptr = PowerWizFireball::GetIcon().lock();
    if (ptr) {
        float w = IMG_RECT.minDim() / 3;
        Rect imgR(0, 0, w, w);
        RenderDataCPtr wImgPtr;
        RenderData wImg;
        RenderData fImg = RenderData(*ptr);
        for (WizardId id : {WIZARD, CRYSTAL, TIME_WIZARD}) {
            switch (id) {
                case WIZARD:
                    imgR.setPos(mPos->rect.x(), mPos->rect.y2(),
                                Rect::Align::CENTER);
                    wImgPtr = WizardDefs::GetIcon().lock();
                    break;
                case CRYSTAL:
                    imgR.setPos(mPos->rect.cX(), mPos->rect.y2(),
                                Rect::Align::CENTER, Rect::Align::TOP_LEFT);
                    wImgPtr = CrystalDefs::GetIcon().lock();
                    break;
                case TIME_WIZARD:
                    imgR.setPos(mPos->rect.x2(), mPos->rect.y2(),
                                Rect::Align::CENTER);
                    wImgPtr = TimeWizardDefs::GetIcon().lock();
                    break;
            }

            if (mStoredFireballs.find(id) == mStoredFireballs.end()) {
                if (wImgPtr) {
                    wImg = RenderData(*wImgPtr);
                    tex.draw(wImg.setDest(imgR));
                }
            } else {
                tex.draw(fImg.setDest(imgR));
            }
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

    auto it = mStoredFireballs.find(target);
    if (it == mStoredFireballs.end() ||
        fireball.getPower() > it->second.power ||
        fireball.getDuration() > it->second.duration) {
        mStoredFireballs[target] = fireball.getData();
        std::cerr << mStoredFireballs[target].speed << std::endl;
    }
}
