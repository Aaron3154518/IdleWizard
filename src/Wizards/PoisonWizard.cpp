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
    mAnimTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            mImg.nextFrame();
            WizardSystem::GetWizardImageObservable()->next(mId, mImg);
            return true;
        },
        PoisonWizardDefs::IMG);
}
void PoisonWizard::setUpgrades() {
    ParameterSystem::Params<POISON_WIZARD> params;
    ParameterSystem::States states;

    UpgradePtr up = std::make_shared<Upgrade>(
        params[PoisonWizardParams::CatGainUp1Lvl], 10);
    up->setImage("");
    up->setDescription({"Multiplies {i} gain *2", {CatalystDefs::GetIcon()}});
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
        {"Improves {i} gain formula", {CatalystDefs::GetIcon()}});
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

void PoisonWizard::onRender(SDL_Renderer* r) { WizardBase::onRender(r); }