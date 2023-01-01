#include "RobotWizard.h"

namespace RobotWizard {
// RobotWizard
RobotWizard::RobotWizard() : WizardBase(ROBOT_WIZARD) {}

void RobotWizard::init() {
    mChargeBtn = ComponentFactory<ChargeButton>::New();

    mImg.set(Constants::IMG());
    mImg.setDest(IMG_RECT);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);

    WizardBase::init();

    if (mUpBot) {
        mUpBot->setPos(mPos->rect.cX(), mPos->rect.cY());
    }

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

    // Multiplier from shards
    UnlockablePtr uUp =
        std::make_shared<Unlockable>(params[Param::BoughtShardPowerUp]);
    uUp->setImage("");
    uUp->setDescription({"Multiplier to {i} from {i} charge",
                         {MoneyIcons::Get(UpgradeDefaults::CRYSTAL_MAGIC),
                          MoneyIcons::Get(UpgradeDefaults::ROBOT_SHARDS)}});
    uUp->setCost(UpgradeDefaults::ROBOT_SHARDS,
                 params[Param::ShardPowerUpCost]);
    uUp->setEffects(params[Param::ShardPowerUp],
                    UpgradeDefaults::MultiplicativeEffect);
    mShardPowerUp = mUpgrades->subscribe(uUp);

    // Wizard crit boost
    uUp = std::make_shared<Unlockable>(params[Param::BoughtWizCritUp]);
    uUp->setImage("");
    uUp->setDescription(
        {"{i} crit is *10^x instead of *x\nUnlocks new {i} "
         "crit upgrade",
         {IconSystem::Get(Wizard::Constants::FB_IMG()),
          IconSystem::Get(Wizard::Constants::IMG())}});
    uUp->setCost(UpgradeDefaults::ROBOT_SHARDS, params[Param::WizCritUpCost]);
    mWizCritUp = mUpgrades->subscribe(uUp);

    // Upgrade bot
    uUp = std::make_shared<Unlockable>(params[Param::UpBotActive]);
    uUp->setImage("");
    uUp->setDescription({"Unlock {i} to automatically purchase wizard upgrades",
                         {IconSystem::Get(Constants::BOT_IMG())}});
    uUp->setCost(UpgradeDefaults::ROBOT_SHARDS, params[Param::UpBotCost]);
    uUp->setEffects({params[Param::UpBotCap], params[Param::UpBotRate]}, {},
                    [params]() -> TextUpdateData {
                        std::stringstream ss;
                        ss << "Capacity: " << params[Param::UpBotCap].get()
                           << "\nCharge Rate: "
                           << params[Param::UpBotRate].get();

                        return {ss.str()};
                    });
    mUpBotUp = mUpgrades->subscribe(uUp);

    // Upgrade bot cap
    uUp = std::make_shared<Unlockable>(params[Param::BoughtUpBotCapRateUp]);
    uUp->setImage("");
    uUp->setDescription(
        {"Multiply {i} capacity by {i} charge\nIncrease charge rate based on "
         "capacity",
         {IconSystem::Get(Constants::BOT_IMG()),
          MoneyIcons::Get(UpgradeDefaults::ROBOT_SHARDS)}});
    uUp->setCost(UpgradeDefaults::ROBOT_SHARDS,
                 params[Param::UpBotCapRateUpCost]);
    uUp->setEffects({params[Param::UpBotCapUp], params[Param::UpBotRateUp]}, {},
                    [params]() -> TextUpdateData {
                        std::stringstream ss;
                        ss << "Capacity "
                           << UpgradeDefaults::MultiplicativeEffectText(
                                  params[Param::UpBotCapUp].get())
                           << "\nCharge Rate +"
                           << UpgradeDefaults::PercentEffectText(
                                  params[Param::UpBotRateUp].get());

                        return {ss.str()};
                    });
    mParamSubs.push_back(params[Param::UpBotCapUp].subscribeTo(
        {UpgradeDefaults::ROBOT_SHARDS}, {uUp->level()}, [params]() {
            return params[Param::BoughtUpBotCapRateUp].get()
                       ? params[Param::Shards].get() + 1
                       : 1;
        }));
    mParamSubs.push_back(params[Param::UpBotRateUp].subscribeTo(
        {params[Param::UpBotCap]}, {uUp->level()}, [params]() {
            return params[Param::BoughtUpBotCapRateUp].get()
                       ? params[Param::UpBotCap].get().logTenCopy() / 25
                       : 0;
        }));
    mUpBotCapRateUp = mUpgrades->subscribe(uUp);

    // Synergy bots - Wizard
    uUp = std::make_shared<Unlockable>(params[Param::WizSynBotActive]);
    uUp->setImage("");
    uUp->setDescription({"Unlock {i} to automatically purchase wizard upgrades",
                         {Constants::BOT_FLOAT_IMG(WIZARD)}});
    uUp->setCost(UpgradeDefaults::ROBOT_SHARDS, params[Param::WizSynBotCost]);
    mWizSynBotUp = mUpgrades->subscribe(uUp);

    // Crystal
    uUp = std::make_shared<Unlockable>(params[Param::CrysSynBotActive]);
    uUp->setImage("");
    uUp->setDescription({"Unlock {i} to automatically purchase wizard upgrades",
                         {Constants::BOT_FLOAT_IMG(CRYSTAL)}});
    uUp->setCost(UpgradeDefaults::ROBOT_SHARDS, params[Param::CrysSynBotCost]);
    mCrysSynBotUp = mUpgrades->subscribe(uUp);

    // Time wizard
    uUp = std::make_shared<Unlockable>(params[Param::TimeWizSynBotActive]);
    uUp->setImage("");
    uUp->setDescription({"Unlock {i} to automatically purchase wizard upgrades",
                         {Constants::BOT_FLOAT_IMG(TIME_WIZARD)}});
    uUp->setCost(UpgradeDefaults::ROBOT_SHARDS,
                 params[Param::TimeWizSynBotCost]);
    mTimeWizSynBotUp = mUpgrades->subscribe(uUp);
}
void RobotWizard::setParamTriggers() {
    Params params;
    Crystal::Params cryParams;
    TimeWizard::Params timeParams;

    mParamSubs.push_back(cryParams[Crystal::Param::BoughtRobotWizard].subscribe(
        [this](bool bought) {
            WizardSystem::GetHideObservable()->next(mId, !bought);
        }));

    mParamSubs.push_back(params[Param::ShardPowerUp].subscribeTo(
        {params[Param::Shards]}, {params[Param::BoughtShardPowerUp]},
        [this]() { return calcShardPowerUp(); }));

    mParamSubs.push_back(
        params[Param::UpBotActive].subscribe([this](bool active) {
            mUpBot = active ? ComponentFactory<UpgradeBot>::New() : nullptr;
        }));

    for (auto pair : Constants::SYN_TARGETS) {
        WizardId id = pair.first;
        mParamSubs.push_back(pair.second.subscribe([this, id](bool active) {
            mSynBots[id] =
                active ? ComponentFactory<SynergyBot>::New(id) : nullptr;
        }));
    }

    mParamSubs.push_back(params[Param::Shards].subscribe([this]() {
        mUpgrades->upgradeThreshhold(UpgradeDefaults::ROBOT_SHARDS);
    }));

    mParamSubs.push_back(params[Param::UpBotCap].subscribeTo(
        {params[Param::UpBotBaseCap], params[Param::UpBotCapUp]}, {},
        [this]() { return calcUpBotCap(); }));

    mParamSubs.push_back(params[Param::UpBotRate].subscribeTo(
        {params[Param::UpBotCap], params[Param::UpBotBaseRate],
         params[Param::UpBotRateUp]},
        {}, [this]() { return calcUpBotRate(); }));
}

void RobotWizard::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    if (mUpgrades->isOpen()) {
        mChargeBtn->onRender(r);
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
void RobotWizard::onResize(ResizeData data) {
    WizardBase::onResize(data);

    mTargetPos = {mTargetPos.x * data.newW / data.oldW,
                  mTargetPos.y * data.newH / data.oldH};
}
void RobotWizard::onHide(bool hide) {
    WizardBase::onHide(hide);

    if (hide) {
        mWaitSub.reset();
    }
}

void RobotWizard::showUpgrades() {
    ServiceSystem::Get<UpgradeService, UpgradeListObservable>()->next(
        mUpgrades, UpgradeDefaults::CRYSTAL_SHARDS, Params::get(Param::Shards));
}

Number RobotWizard::calcShardPowerUp() {
    Params params;

    return params[Param::BoughtShardPowerUp].get()
               ? params[Param::Shards].get() + 1
               : 1;
}
Number RobotWizard::calcUpBotCap() {
    Params params;

    return params[Param::UpBotBaseCap].get() * params[Param::UpBotCapUp].get();
}
Number RobotWizard::calcUpBotRate() {
    Params params;

    return params[Param::UpBotCap].get() * (params[Param::UpBotBaseRate].get() +
                                            params[Param::UpBotRateUp].get());
}
}  // namespace RobotWizard
