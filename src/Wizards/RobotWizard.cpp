#include "RobotWizard.h"

// Portals
RobotWizard::Portals::Portals()
    : mPortalTopPos(
          std::make_shared<UIComponent>(Rect(), Elevation::PORTAL_TOP)),
      mPortalBotPos(
          std::make_shared<UIComponent>(Rect(), Elevation::PORTAL_BOT)) {
    mPortalTopImg.set(RobotWizardDefs::PORTAL_TOP);
    mPortalBotImg.set(RobotWizardDefs::PORTAL_BOT);

    mPortalTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            [this](Timer& t) {
                mPortalBotImg.nextFrame();
                mPortalTopImg.nextFrame();
                if (mPortalBotImg.getFrame() == 0) {
                    setActive(false);
                }
                return true;
            },
            RobotWizardDefs::PORTAL_TOP.frame_ms);

    mPortalTopPos->mouse = mPortalBotPos->mouse = false;
    mPortalTopRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [this](SDL_Renderer* r) { TextureBuilder().draw(mPortalTopImg); },
            mPortalTopPos);
    mPortalBotRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [this](SDL_Renderer* r) { TextureBuilder().draw(mPortalBotImg); },
            mPortalBotPos);

    setActive(false);
}

void RobotWizard::Portals::start(const Rect& r) {
    mPortalTopImg.setFrame(0).setDest(r);
    mPortalBotImg.setFrame(0).setDest(r);
    setActive(true);
}

void RobotWizard::Portals::setActive(bool active) {
    mPortalTopRenderSub->setActive(active);
    mPortalBotRenderSub->setActive(active);
    mPortalTimerSub->setActive(active);
}

// RobotWizard
RobotWizard::RobotWizard() : WizardBase(ROBOT_WIZARD) {}

void RobotWizard::init() {
    mImg.set(RobotWizardDefs::IMG).setDest(IMG_RECT);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);

    WizardBase::init();

    mDragSub.reset();

    mTargetPos = {mPos->rect.cX(), mPos->rect.cY()};
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
        [this](Time dt) { onMoveUpdate(dt); });
    mUpTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) { return onUpgradeTimer(t); }, Timer(1000));

    attachSubToVisibility(mPowFireballHitSub);
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
            if (!ParameterSystem::Param(State::BoughtRobotWizard).get()) {
                return;
            }

            std::vector<WizardId> targets;
            if (frozen) {
                targets = {WIZARD};
            } else {
                targets = {CRYSTAL, TIME_WIZARD};
            }

            for (auto target : targets) {
                auto it = mStoredFireballs.find(target);
                if (it == mStoredFireballs.end()) {
                    continue;
                }
                Rect pos = WizardSystem::GetWizardPos(it->first);
                SDL_FPoint p{
                    pos.cX() - (mPos->rect.w() * (rDist(gen) * .5f + .5f)),
                    pos.cY() + (mPos->rect.h() * (rDist(gen) * 1.f - .5f))};
                mFireballs.push_back(ComponentFactory<PowerWizFireball>::New(
                    p, target, it->second));
                mStoredFireballs.erase(it);

                // Setup portals
                float w = mPos->rect.minDim();
                Rect r(0, 0, w, w);
                r.setPos(p.x, p.y, Rect::Align::CENTER);
                mPortals[it->first].start(r);
            }
        }));

    mParamSubs.push_back(states[State::ShootRobot].subscribeTo(
        {}, {states[State::BoughtRobotWizard]}, []() {
            ParameterSystem::States states;
            return states[State::BoughtRobotWizard].get();
        }));
}

void RobotWizard::onMoveUpdate(Time dt) {
    bool wizTarget = mTarget != WizardId::size;

    if (wizTarget) {
        Rect r = WizardSystem::GetWizardPos(mTarget);
        mTargetPos = {r.cX(), r.cY()};
    }
    float dx = mTargetPos.x - mPos->rect.cX(),
          dy = mTargetPos.y - mPos->rect.cY();
    float mag = sqrtf(dx * dx + dy * dy);

    if (mag <= (wizTarget ? 50 : 10)) {
        if (wizTarget) {
            upgradeTarget();
        }
        mTarget = WizardId::size;
        mWaitSub =
            ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
                [this](Timer& t) {
                    SDL_Point dim = RenderSystem::getWindowSize();
                    float dx = (rDist(gen) * .25f + .1f) * dim.x,
                          dy = (rDist(gen) * .25f + .1f) * dim.y;
                    mTargetPos.x += (mTargetPos.x < dim.x / 2) ? dx : -dx;
                    mTargetPos.y += (mTargetPos.y < dim.y / 2) ? dy : -dy;
                    mMoveUpdateSub->setActive(true);
                    return false;
                },
                Timer(rDist(gen) * 4000 + 2500));
        mMoveUpdateSub->setActive(false);
    } else {
        float frac = 100 * dt.s() / mag;
        setPos(mPos->rect.cX() + dx * frac, mPos->rect.cY() + dy * frac);
    }
}
bool RobotWizard::onUpgradeTimer(Timer& t) {
    if (mTarget == WizardId::size && rDist(gen) < .1) {
        mTargetIdx = (mTargetIdx + 1) % RobotWizardDefs::TARGETS.size();
        mTarget = RobotWizardDefs::TARGETS.at(mTargetIdx);
        mWaitSub.reset();
        mMoveUpdateSub->setActive(true);
        /*do {
            mTargetIdx = (mTargetIdx + 1) % RobotWizardDefs::TARGETS.size();
        } while (RobotWizardDefs::TARGETS.at(mTargetIdx) != CRYSTAL &&
                 WizardSystem::Hidden(RobotWizardDefs::TARGETS.at(mTargetIdx)));*/
    }
    return true;
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

    for (auto it = mFireballs.begin(); it != mFireballs.end(); ++it) {
        if ((*it)->dead()) {
            it = mFireballs.erase(it);
            if (it == mFireballs.end()) {
                break;
            }
        }
    }
}
void RobotWizard::onResize(ResizeData data) {
    WizardBase::onResize(data);

    mTargetPos = {mTargetPos.x * data.newW / data.oldW,
                  mTargetPos.y * data.newH / data.oldH};
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

void RobotWizard::upgradeTarget() {
    auto catMagic = ParameterSystem::Param<CATALYST>(CatalystParams::Magic);
    Number maxSpend = catMagic.get() / 10 + 1;
    Number spent = GetWizardUpgrades(mTarget)->buyAll(
        UpgradeDefaults::CRYSTAL_MAGIC, catMagic.get() / 10);
    catMagic.set(catMagic.get() - spent);
}
