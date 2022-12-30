#include "RobotWizard.h"

namespace RobotWizard {
// Portals
RobotWizard::Portals::Portals()
    : mPortalTopPos(
          std::make_shared<UIComponent>(Rect(), Elevation::PORTAL_TOP)),
      mPortalBotPos(
          std::make_shared<UIComponent>(Rect(), Elevation::PORTAL_BOT)) {
    mPortalTopImg.set(Constants::PORTAL_TOP());
    mPortalBotImg.set(Constants::PORTAL_BOT());

    mPortalTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            [this](Timer& t) {
                mPortalBotImg->nextFrame();
                mPortalTopImg->nextFrame();
                if (mPortalBotImg->getFrame() == 0) {
                    setActive(false);
                }
                return true;
            },
            Constants::PORTAL_TOP().frame_ms);

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
    mPortalTopImg->setFrame(0);
    mPortalTopImg.setDest(r);
    mPortalBotImg->setFrame(0);
    mPortalBotImg.setDest(r);
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
    mFireballs = ComponentFactory<PowerWizard::RobotFireballList>::New();
    mUpBot = ComponentFactory<UpgradeBot>::New();

    mImg.set(Constants::IMG());
    mImg.setDest(IMG_RECT);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);

    WizardBase::init();

    mUpBot->setPos(mPos->rect.cX(), mPos->rect.cY());

    mDragSub.reset();

    mTargetPos = {mPos->rect.cX(), mPos->rect.cY()};
}
void RobotWizard::setSubscriptions() {
    mAnimTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            [this](Timer& t) {
                mImg->nextFrame();
                WizardSystem::GetWizardImageObservable()->next(mId, mImg);
                return true;
            },
            Constants::IMG());
    mMoveUpdateSub = TimeSystem::GetUpdateObservable()->subscribe(
        [this](Time dt) { onMoveUpdate(dt); });

    attachSubToVisibility(mMoveUpdateSub);
}
void RobotWizard::setUpgrades() {
    Params params;

    UnlockablePtr uUp =
        std::make_shared<Unlockable>(params[Param::BoughtWizCritUp]);
    uUp->setImage("");
    uUp->setDescription(
        {"{i} crit is *10^crit instead of *crit\nUnlocks new {i} "
         "crit upgrade",
         {IconSystem::Get(Wizard::Constants::FB_IMG()),
          IconSystem::Get(Wizard::Constants::IMG())}});
    uUp->setCost(UpgradeDefaults::CRYSTAL_SHARDS, params[Param::WizCritUpCost]);
    mWizCritUp = mUpgrades->subscribe(uUp);

    using Param::P_B;
    params[P_B::U1]->init(0);
    params[P_B::U2]->init(0.1);
    params[P_B::U3]->init(0.5);
    params[P_B::U4]->init(1);
    params[P_B::U5]->init(5);
    params[P_B::U6]->init(100);
    params[P_B::U7]->init(100);
    params[P_B::U8]->init(Number(1, 10));
    params[P_B::U9]->init(Number(1, 5));
    params[P_B::U10]->init(Number(5, 15));
    int i = 0;
    for (auto p : {P_B::U1, P_B::U2, P_B::U3, P_B::U4, P_B::U5, P_B::U6,
                   P_B::U7, P_B::U8, P_B::U9, P_B::U10}) {
        uUp = std::make_shared<Unlockable>(params[Param::BoughtWizCritUp]);
        uUp->setImage(p == P_B::U7 ? Constants::IMG().file : "");
        uUp->setDescription({"{i} " + std::to_string(i++),
                             {IconSystem::Get(Constants::IMG())}});
        uUp->setCost(UpgradeDefaults::CRYSTAL_SHARDS, params[p]);
        mUps.push_back(mUpgrades->subscribe(uUp));
    }
}
void RobotWizard::setParamTriggers() {
    Params params;
    Crystal::Params cryParams;
    TimeWizard::Params timeParams;

    mParamSubs.push_back(cryParams[Crystal::Param::BoughtRobotWizard].subscribe(
        [this](bool bought) {
            WizardSystem::GetHideObservable()->next(mId, !bought);
        }));

    mParamSubs.push_back(timeParams[TimeWizard::Param::Frozen].subscribe(
        [this](bool frozen) {
            if (!Crystal::Params::get(Crystal::Param::BoughtRobotWizard)
                     .get()) {
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
                mFireballs->push_back(
                    ComponentFactory<PowerWizard::Fireball>::New(p, target,
                                                                 it->second));
                mStoredFireballs.erase(it);

                // Setup portals
                float w = mPos->rect.minDim();
                Rect r(0, 0, w, w);
                r.setPos(p.x, p.y, Rect::Align::CENTER);
                mPortals[it->first].start(r);
            }
        }));

    for (auto pair : Constants::SYN_TARGETS) {
        WizardId id = pair.first;
        mParamSubs.push_back(pair.second.subscribe([this, id](bool active) {
            mSynBots[id] =
                active ? ComponentFactory<SynergyBot>::New(id) : nullptr;
        }));
    }
}

void RobotWizard::onMoveUpdate(Time dt) {
    float dx = mTargetPos.x - mPos->rect.cX(),
          dy = mTargetPos.y - mPos->rect.cY();
    float mag = sqrtf(dx * dx + dy * dy);

    if (mag <= 1e-5) {
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
                Timer(rDist(gen) * 6000 + 4000));
        mMoveUpdateSub->setActive(false);
    } else {
        float frac = 100 * dt.s() / mag;
        setPos(mPos->rect.cX() + dx * fminf(frac, 1),
               mPos->rect.cY() + dy * fminf(frac, 1));
    }
}
void RobotWizard::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    TextureBuilder tex;

    RenderData fImg, wImg;
    fImg.set(IconSystem::Get(PowerWizard::Constants::FB_IMG()));
    float w = IMG_RECT.minDim() / 3;
    Rect imgR(0, 0, w, w);
    for (WizardId id : {WIZARD, CRYSTAL, TIME_WIZARD}) {
        switch (id) {
            case WIZARD:
                imgR.setPos(mPos->rect.x(), mPos->rect.y2(),
                            Rect::Align::CENTER);
                wImg.set(IconSystem::Get(Wizard::Constants::IMG()));
                break;
            case CRYSTAL:
                imgR.setPos(mPos->rect.cX(), mPos->rect.y2(),
                            Rect::Align::CENTER, Rect::Align::TOP_LEFT);
                wImg.set(IconSystem::Get(Crystal::Constants::IMG()));
                break;
            case TIME_WIZARD:
                imgR.setPos(mPos->rect.x2(), mPos->rect.y2(),
                            Rect::Align::CENTER);
                wImg.set(IconSystem::Get(TimeWizard::Constants::IMG()));
                break;
        }

        if (mStoredFireballs.find(id) == mStoredFireballs.end()) {
            wImg.setDest(imgR);
            tex.draw(wImg);
        } else {
            fImg.setDest(imgR);
            tex.draw(fImg);
        }
    }
}
void RobotWizard::onResize(ResizeData data) {
    WizardBase::onResize(data);

    mTargetPos = {mTargetPos.x * data.newW / data.oldW,
                  mTargetPos.y * data.newH / data.oldH};
}
void RobotWizard::onHide(bool hide) {
    WizardBase::onHide(hide);

    if (hide) {
        mWaitSub.reset();
    } else {
        // mUpTimerSub->setActive(false);
    }
}

void RobotWizard::showUpgrades() {
    ServiceSystem::Get<UpgradeService, UpgradeListObservable>()->next(
        mUpgrades, UpgradeDefaults::CRYSTAL_SHARDS,
        Params::get(Param::ShardAmnt));
}
}  // namespace RobotWizard
