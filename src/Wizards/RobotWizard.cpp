#include "RobotWizard.h"

// RobotWizard
RobotWizard::RobotWizard()
    : WizardBase(ROBOT_WIZARD),
      mPortalTopPos(
          std::make_shared<UIComponent>(Rect(), Elevation::PORTAL_TOP)),
      mPortalBotPos(
          std::make_shared<UIComponent>(Rect(), Elevation::PORTAL_BOT)) {
    mPortalTopPos->mouse = mPortalBotPos->mouse = false;
}

void RobotWizard::init() {
    mImg.set(RobotWizardDefs::IMG).setDest(IMG_RECT);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);

    mPortalTopImg.set(RobotWizardDefs::PORTAL_TOP);
    mPortalBotImg.set(RobotWizardDefs::PORTAL_BOT);

    WizardBase::init();

    mDragSub.reset();
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

    // Portal stuff
    mPortalTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            [this](Timer& t) {
                mPortalBotImg.nextFrame();
                mPortalTopImg.nextFrame();
                if (mPortalBotImg.getFrame() == 0) {
                    mPortalTimerSub->setActive(false);
                    mPortalTopRenderSub->setActive(false);
                    mPortalBotRenderSub->setActive(false);
                }
                return true;
            },
            RobotWizardDefs::PORTAL_TOP.frame_ms);
    mPortalTopRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [this](SDL_Renderer* r) { TextureBuilder().draw(mPortalTopImg); },
            mPortalTopPos);
    mPortalBotRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [this](SDL_Renderer* r) { TextureBuilder().draw(mPortalBotImg); },
            mPortalBotPos);
    mPortalTimerSub->setActive(false);
    mPortalTopRenderSub->setActive(false);
    mPortalBotRenderSub->setActive(false);

    // mUpTimerSub = TimeSystem::GetTimerObservable()->subscribe(
    //[this](Timer& t) { return onUpTimer(t); }, Timer(2000));

    attachSubToVisibility(mPowFireballHitSub);
    attachSubToVisibility(mTpUpdateSub);
    attachSubToVisibility(mMoveUpdateSub);
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

    mParamSubs.push_back(states[State::ShootRobot].subscribeTo(
        {}, {states[State::BoughtRobotWizard]}, []() {
            ParameterSystem::States states;
            return states[State::BoughtRobotWizard].get();
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
            SDL_FPoint p{
                pos.cX() + (mPos->rect.w() * (rDist(gen) * .5f + .5f)),
                pos.cY() + (mPos->rect.h() * (rDist(gen) * .5f + .5f))};
            mFireballs.push_back(ComponentFactory<PowerWizFireball>::New(
                p, it->first, it->second));
            mTpQueue.pop();
            mStoredFireballs.erase(it);

            // Setup portals
            float w = mPos->rect.minDim();
            Rect r(0, 0, w, w);
            r.setPos(p.x, p.y, Rect::Align::CENTER);
            mPortalTopImg.setFrame(0).setDest(r);
            mPortalBotImg.setFrame(0).setDest(r);
            mPortalTimerSub->setActive(true);
            mPortalTopRenderSub->setActive(true);
            mPortalBotRenderSub->setActive(true);
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
        mStoredFireballs[target].src = ROBOT_WIZARD;
    }
}
