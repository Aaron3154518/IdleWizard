#include "PoisonWizard.h"

// PoisonWizard
PoisonWizard::PoisonWizard() : WizardBase(POISON_WIZARD) {}

void PoisonWizard::init() {
    mFireballs = ComponentFactory<PoisonFireballList>::New();

    mImg.set(PoisonWizardDefs::IMG);
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
        PoisonWizardDefs::IMG);
    mT1ResetSub = WizardSystem::GetWizardEventObservable()->subscribe(
        [this]() { onT1Reset(); }, WizardSystem::Event::ResetT1);
    attachSubToVisibility(mFireballTimerSub);
}
void PoisonWizard::setUpgrades() {
    ParameterSystem::Params<POISON_WIZARD> params;
    ParameterSystem::Params<CRYSTAL> cryParams;
    ParameterSystem::States states;

    // Poison gain
    UnlockablePtr uUp =
        std::make_shared<Unlockable>(states[State::CrysPoisonActive]);
    uUp->setImage("");
    uUp->setDescription(
        {"{i} gains 10% of best {i} over time",
         {IconSystem::Get(CrystalDefs::IMG),
          Money::GetMoneyIcon(UpgradeDefaults::CRYSTAL_MAGIC)}});
    uUp->setCost(UpgradeDefaults::CRYSTAL_SHARDS,
                 params[PoisonWizardParams::CrysPoisonUpCost]);
    uUp->setEffects(
        {cryParams[CrystalParams::PoisonMagic],
         cryParams[CrystalParams::PoisonRate]},
        {uUp->level()}, [states, cryParams]() -> TextUpdateData {
            std::stringstream ss;
            ss << UpgradeDefaults::PercentEffectText(
                      cryParams[CrystalParams::PoisonRate].get())
               << " of " << cryParams[CrystalParams::PoisonMagic].get()
               << "{i}/s";
            return {ss.str(),
                    {Money::GetMoneyIcon(UpgradeDefaults::CRYSTAL_MAGIC)}};
        });
    mParamSubs.push_back(params[PoisonWizardParams::BasePoisonRate].subscribeTo(
        uUp->level(), [](const bool& bought) { return bought ? 0.1 : 0; }));
    mCrysPoisonUp = mUpgrades->subscribe(uUp);

    UpgradePtr up =
        std::make_shared<Upgrade>(params[PoisonWizardParams::PoisonFbUpLvl], 5);
    up->setImage("");
    up->setDescription({"{i} increase the {i} gain rate by +10%/lvl",
                        {IconSystem::Get(WizardDefs::FB_POISON_IMG),
                         Money::GetMoneyIcon(UpgradeDefaults::CRYSTAL_MAGIC)}});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                params[PoisonWizardParams::PoisonFbUpCost]);
    up->setEffects(params[PoisonWizardParams::PoisonFbUp],
                   UpgradeDefaults::PercentEffect);
    mParamSubs.push_back(params[PoisonWizardParams::PoisonFbUpCost].subscribeTo(
        up->level(),
        [](const Number& lvl) { return Number(1, 15) * (4.5 ^ lvl); }));
    mParamSubs.push_back(params[PoisonWizardParams::PoisonFbUp].subscribeTo(
        up->level(), [](const Number& lvl) { return lvl / 10; }));
    mPoisonFbUp = mUpgrades->subscribe(up);
}
void PoisonWizard::setParamTriggers() {
    ParameterSystem::Params<POISON_WIZARD> params;
    ParameterSystem::States states;

    mParamSubs.push_back(params[PoisonWizardParams::Speed].subscribeTo(
        {params[PoisonWizardParams::BaseSpeed]}, {},
        [this]() { return calcSpeed(); }));

    mParamSubs.push_back(
        params[PoisonWizardParams::Speed].subscribe([this]() { calcTimer(); }));

    mParamSubs.push_back(
        states[State::BoughtPoisonWizard].subscribe([this](bool bought) {
            WizardSystem::GetHideObservable()->next(mId, !bought);
        }));
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
    ParameterSystem::Params<POISON_WIZARD> params;
    return params[PoisonWizardParams::BaseSpeed].get();
}
void PoisonWizard::calcTimer() {
    ParameterSystem::Params<POISON_WIZARD> params;

    Timer& timer = mFireballTimerSub->get<TimerObservable::DATA>();
    float div = params[PoisonWizardParams::Speed].get().toFloat();
    if (div <= 0) {
        div = 1;
    }
    float prevLen = timer.length;
    timer.length = fmax(1000 / div, 16);
    timer.timer *= timer.length / prevLen;
}

void PoisonWizard::shootFireball() {
    auto data = newFireballData();
    mFireballs->push_back(ComponentFactory<PoisonFireball>::New(
        SDL_FPoint{mPos->rect.cX(), mPos->rect.cY()}, ROBOT_WIZARD, data));

    for (int i = 0; i < 25; i++) {
        float theta = rDist(gen) * 2 * M_PI;
        mGlobs.push_back(ComponentFactory<Glob>::New(
            mPos->rect.getPos(Rect::Align::CENTER),
            SDL_FPoint{cosf(theta) * 500, sinf(theta) * 500}));
    }
}

PoisonFireball::Data PoisonWizard::newFireballData() {
    ParameterSystem::Params<WIZARD> params;

    PoisonFireball::Data data;
    data.duration = 7500;

    return data;
}
