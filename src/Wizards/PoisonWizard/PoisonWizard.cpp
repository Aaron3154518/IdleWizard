#include "PoisonWizard.h"

namespace PoisonWizard {
// PoisonWizard
PoisonWizard::PoisonWizard() : WizardBase(POISON_WIZARD) {}

void PoisonWizard::init() {
    mFireballs = ComponentFactory<FireballList>::New();

    mImg.set(Constants::IMG());
    mImg.setDest(IMG_RECT);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);

    WizardBase::init();
}
void PoisonWizard::setSubscriptions() {
    mFireballTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            [this](Timer& t) { return onFireballTimer(t); }, Timer(1000));
    mAnimTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            mImg->nextFrame();
            WizardSystem::GetWizardImageObservable()->next(mId, mImg);
            return true;
        },
        Constants::IMG());
    mT1ResetSub = WizardSystem::GetWizardEventObservable()->subscribe(
        [this]() { onT1Reset(); }, WizardSystem::Event::ResetT1);
    attachSubToVisibility(mFireballTimerSub);
}
void PoisonWizard::setUpgrades() {
    PoisonWizard::Params params;
    Crystal::Params cryParams;

    // Poison gain
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(mId);
    dUp->setDescription(
        {"{i} gains % of best {i} gain over time",
         {IconSystem::Get(Crystal::Constants::IMG()),
          MoneyIcons::GetMoneyIcon(UpgradeDefaults::CRYSTAL_MAGIC)}});
    dUp->setEffects(
        {cryParams[Crystal::Param::PoisonMagic],
         cryParams[Crystal::Param::PoisonRate]},
        {}, [states, cryParams]() -> TextUpdateData {
            std::stringstream ss;
            ss << UpgradeDefaults::PercentEffectText(
                      cryParams[Crystal::Param::PoisonRate].get())
               << " of " << cryParams[Crystal::Param::PoisonMagic].get()
               << "{i}/s";
            return {ss.str(),
                    {MoneyIcons::GetMoneyIcon(UpgradeDefaults::CRYSTAL_MAGIC)}};
        });
    mPoisonDisplay = mUpgrades->subscribe(dUp);

    // Shard multiplier
    UpgradePtr up = std::make_shared<Upgrade>(
        params[PoisonWizard::Param::ShardMultUpLvl], 5);
    up->setImage("");
    up->setDescription(
        {"Triple {i} gain",
         {MoneyIcons::GetMoneyIcon(UpgradeDefaults::CRYSTAL_MAGIC)}});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                params[PoisonWizard::Param::ShardMultUpCost]);
    up->setEffects(params[PoisonWizard::Param::ShardMultUp],
                   UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(
        params[PoisonWizard::Param::ShardMultUpCost].subscribeTo(
            up->level(), [](const Number& lvl) {
                return Number(1, 20 + 10 * lvl.toInt());
            }));
    mParamSubs.push_back(params[PoisonWizard::Param::ShardMultUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 3 ^ lvl; }));
    mShardMultUp = mUpgrades->subscribe(up);

    // Increase poison gain from poison fbs
    up =
        std::make_shared<Upgrade>(params[PoisonWizard::Param::PoisonFbUpLvl], 5);
    up->setImage("");
    up->setDescription(
        {"{i} increase the {i} gain rate by +10%/lvl",
         {IconSystem::Get(Wizard::Constants::FB_POISON_IMG()),
          MoneyIcons::GetMoneyIcon(UpgradeDefaults::CRYSTAL_MAGIC)}});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                params[PoisonWizard::Param::PoisonFbUpCost]);
    up->setEffects(params[PoisonWizard::Param::PoisonFbUp],
                   UpgradeDefaults::PercentEffect);
    mParamSubs.push_back(params[PoisonWizard::Param::PoisonFbUpCost].subscribeTo(
        up->level(),
        [](const Number& lvl) { return Number(1, 15) * (4.5 ^ lvl); }));
    mParamSubs.push_back(params[PoisonWizard::Param::PoisonFbUp].subscribeTo(
        up->level(), [](const Number& lvl) { return lvl / 10; }));
    mPoisonFbUp = mUpgrades->subscribe(up);

    // Increase globs fired
    up = std::make_shared<Upgrade>(params[PoisonWizard::Param::GlobCntUpLvl], 3);
    up->setImage("");
    up->setDescription({"Increases {i} count by +2",
                        {IconSystem::Get(Constants::GLOB_IMG())}});
    up->setCost(UpgradeDefaults::CRYSTAL_SHARDS,
                params[PoisonWizard::Param::GlobCntCost]);
    up->setEffects(params[PoisonWizard::Param::GlobCntUp],
                   UpgradeDefaults::AdditiveEffect);
    mParamSubs.push_back(params[PoisonWizard::Param::GlobCntCost].subscribeTo(
        up->level(), [](const Number& lvl) { return 50 * (1.5 ^ lvl); }));
    mParamSubs.push_back(params[PoisonWizard::Param::GlobCntUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 2 * lvl; }));
    mGlobCntUp = mUpgrades->subscribe(up);

    // Shoot poison ball at catalyst
    UnlockablePtr uUp =
        std::make_shared<Unlockable>(states[State::BoughtPoisWizCatPois]);
    uUp->setImage("");
    uUp->setDescription(
        {"Fires {i} at {i} to grant poison effect. {i} zaps convert {i} into "
         "{i}",
         {IconSystem::Get(Constants::GLOB_IMG()),
          IconSystem::Get(Catalyst::Constants::IMG()),
          IconSystem::Get(Catalyst::Constants::IMG()),
          IconSystem::Get(Wizard::Constants::FB_IMG()),
          IconSystem::Get(Wizard::Constants::FB_POISON_IMG())}});
    uUp->setCost(UpgradeDefaults::CATALYST_MAGIC,
                 params[PoisonWizard::Param::CatPoisonUpCost]);
    mCatPoisUp = mUpgrades->subscribe(uUp);
}
void PoisonWizard::setParamTriggers() {
    PoisonWizard::Params params;

    mParamSubs.push_back(params[PoisonWizard::Param::Speed].subscribeTo(
        {params[PoisonWizard::Param::BaseSpeed]}, {},
        [this]() { return calcSpeed(); }));

    mParamSubs.push_back(
        params[PoisonWizard::Param::Speed].subscribe([this]() { calcTimer(); }));

    mParamSubs.push_back(params[PoisonWizard::Param::GlobCnt].subscribeTo(
        {params[PoisonWizard::Param::BaseGlobCnt],
         params[PoisonWizard::Param::GlobCntUp]},
        {}, [this]() { return calcBlobCount(); }));

    mParamSubs.push_back(
        states[Crystal::Param::BoughtPoisonWizard].subscribe([this](bool bought) {
            WizardSystem::GetHideObservable()->next(mId, !bought);
        }));

    mParamSubs.push_back(ParameterSystem::subscribe(
        {},
        {states[State::BoughtPoisWizCatPois], states[Crystal::Param::BoughtCatalyst]},
        [this]() { setTargets(); }));
}

void PoisonWizard::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    for (auto it = mGlobs.begin(); it != mGlobs.end();) {
        if ((*it)->dead()) {
            it = mGlobs.erase(it);
        } else {
            ++it;
        }
    }
}
void PoisonWizard::onHide(bool hide) {
    WizardBase::onHide(hide);

    mFireballs->clear();
    mGlobs.clear();
}
bool PoisonWizard::onFireballTimer(Timer& t) {
    shootFireball();
    return true;
}
void PoisonWizard::onT1Reset() {
    mFireballs->clear();

    if (mFireballTimerSub) {
        mFireballTimerSub->get<TimerObservable::DATA>().reset();
    }
}

Number PoisonWizard::calcSpeed() {
    PoisonWizard::Params params;
    return params[PoisonWizard::Param::BaseSpeed].get();
}
void PoisonWizard::calcTimer() {
    PoisonWizard::Params params;

    Timer& timer = mFireballTimerSub->get<TimerObservable::DATA>();
    float div = params[PoisonWizard::Param::Speed].get().toFloat();
    if (div <= 0) {
        div = 1;
    }
    float prevLen = timer.length;
    timer.length = fmax(1000 / div, 16);
    timer.timer *= timer.length / prevLen;
}
Number PoisonWizard::calcBlobCount() {
    PoisonWizard::Params params;
    return params[PoisonWizard::Param::BaseGlobCnt].get() +
           params[PoisonWizard::Param::GlobCntUp].get();
}

void PoisonWizard::setTargets() {

    mTargets.clear();
    if (states[State::BoughtPoisWizCatPois].get() &&
        states[Crystal::Param::BoughtCatalyst].get()) {
        mTargets.push_back(CATALYST);
    }
}

void PoisonWizard::shootFireball() {
    PoisonWizard::Params params;

    if (!mTargets.empty()) {
        auto data = newFireballData();
        mFireballs->push_back(ComponentFactory<Fireball>::New(
            SDL_FPoint{mPos->rect.cX(), mPos->rect.cY()},
            mTargets.at((int)(rDist(gen) * mTargets.size())), data));
    }

    Number cnt = params[PoisonWizard::Param::GlobCnt].get();
    for (int i = 0; i < cnt; i++) {
        float theta = rDist(gen) * 2 * M_PI;
        mGlobs.push_back(ComponentFactory<Glob>::New(
            mPos->rect.getPos(Rect::Align::CENTER),
            SDL_FPoint{cosf(theta) * 500, sinf(theta) * 500}));
    }
}

Fireball::Data PoisonWizard::newFireballData() {
    Wizard::Params params;

    Fireball::Data data;
    data.duration = 7500;

    return data;
}
}  // namespace PoisonWizard
