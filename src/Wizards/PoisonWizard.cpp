#include "PoisonWizard.h"

// PoisonWizard
PoisonWizard::PoisonWizard() : WizardBase(POISON_WIZARD) {}

void PoisonWizard::init() {
    mImg.set(PoisonWizardDefs::IMG).setDest(IMG_RECT);
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
            mImg.nextFrame();
            WizardSystem::GetWizardImageObservable()->next(mId, mImg);
            return true;
        },
        PoisonWizardDefs::IMG);
    mT1ResetSub = WizardSystem::GetWizardEventObservable()->subscribe(
        [this]() { onT1Reset(); }, WizardSystem::Event::ResetT1);
}
void PoisonWizard::setUpgrades() {
    ParameterSystem::Params<POISON_WIZARD> params;
    ParameterSystem::States states;

    UpgradePtr up = std::make_shared<Upgrade>(
        params[PoisonWizardParams::CatGainUp1Lvl], 10);
    up->setImage("");
    up->setDescription(
        {"Multiplies {i} gain *2", {IconSystem::Get(CatalystDefs::IMG)}});
    up->setCost(UpgradeDefaults::CRYSTAL_SHARDS,
                params[PoisonWizardParams::CatGainUp1Cost]);
    up->setEffects(params[PoisonWizardParams::CatGainUp1],
                   UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(params[PoisonWizardParams::CatGainUp1Cost].subscribeTo(
        up->level(), [](const Number& lvl) { return 10 * (2 ^ lvl); }));
    mParamSubs.push_back(params[PoisonWizardParams::CatGainUp1].subscribeTo(
        up->level(), [](const Number& lvl) { return 2 ^ lvl; }));
    mCatGainUp1 = mUpgrades->subscribe(up);

    up =
        std::make_shared<Upgrade>(params[PoisonWizardParams::CatGainUp2Lvl], 5);
    up->setImage("");
    up->setDescription(
        {"Improves {i} gain formula", {IconSystem::Get(CatalystDefs::IMG)}});
    up->setCost(UpgradeDefaults::CRYSTAL_SHARDS,
                params[PoisonWizardParams::CatGainUp2Cost]);
    up->setEffects(
        params[PoisonWizardParams::CatGainUp2],
        [](const Number& effect) -> TextUpdateData {
            return {"log(x) -> x" + UpgradeDefaults::PowerEffect(effect).text};
        });
    mParamSubs.push_back(params[PoisonWizardParams::CatGainUp2Cost].subscribeTo(
        up->level(),
        [](const Number& lvl) { return Number(1, 4) * (5 ^ lvl); }));
    mParamSubs.push_back(params[PoisonWizardParams::CatGainUp2].subscribeTo(
        up->level(), [](const Number& lvl) { return lvl / 10; }));
    mCatGainUp2 = mUpgrades->subscribe(up);
}
void PoisonWizard::setParamTriggers() {
    ParameterSystem::Params<POISON_WIZARD> params;
    ParameterSystem::States states;

    mParamSubs.push_back(
        states[State::BoughtPoisonWizard].subscribe([this](bool bought) {
            WizardSystem::GetHideObservable()->next(mId, !bought);
        }));

    mParamSubs.push_back(params[PoisonWizardParams::CatGainUp1Lvl].subscribe(
        [this](const Number& lvl) {
            mCatGainUp2->setActive(
                mCatGainUp1->get<UpgradeList::DATA>()->status() ==
                Upgrade::BOUGHT);
        }));
}

void PoisonWizard::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    for (auto it = mFireballs.begin(); it != mFireballs.end(); ++it) {
        if ((*it)->dead()) {
            it = mFireballs.erase(it);
            if (it == mFireballs.end()) {
                break;
            }
        }
    }
}
bool PoisonWizard::onFireballTimer(Timer& t) {
    shootFireball();
    return true;
}
void PoisonWizard::onT1Reset() {
    mFireballs.clear();

    if (mFireballTimerSub) {
        mFireballTimerSub->get<TimerObservable::DATA>().reset();
    }
}

void PoisonWizard::shootFireball() {
    auto data = newFireballData();
    float rand = rDist(gen) * 3;
    mFireballs.push_back(ComponentFactory<PoisonFireball>::New(
        SDL_FPoint{mPos->rect.cX(), mPos->rect.cY()},
        rand < 1   ? CRYSTAL
        : rand < 2 ? CATALYST
                   : ROBOT_WIZARD,
        data));
}

PoisonFireball::Data PoisonWizard::newFireballData() {
    ParameterSystem::Params<WIZARD> params;

    return {};
}
