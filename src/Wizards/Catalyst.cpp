#include "Catalyst.h"

// Catalyst
const unsigned int Catalyst::MSPF = 150, Catalyst::NUM_FRAMES = 5;

const std::string Catalyst::IMG = "res/wizards/catalyst.png";

void Catalyst::setDefaults() {
    using WizardSystem::ResetTier;

    ParameterSystem::Params<CATALYST> params;

    params[CatalystParams::Magic]->init(0);
    params[CatalystParams::Capacity]->init(100);
    params[CatalystParams::BaseRange]->init(1.25);

    params[CatalystParams::RangeUpLvl]->init(ResetTier::T2);
}

Catalyst::Catalyst() : WizardBase(CATALYST) {
    mPos->elevation = Elevation::CATALYST;
}

void Catalyst::init() {
    mImg.set(IMG).setDest(IMG_RECT);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);

    mMagicText.font = AssetManager::getFont(FONT);
    mMagicRender.setFit(RenderData::FitMode::Texture);

    mRange = CircleShape(PURPLE).setDashed(50);

    setPos(mPos->rect.cX(), mPos->rect.cY());

    WizardBase::init();
}
void Catalyst::setSubscriptions() {
    mWizFireballSub = WizardFireball::GetHitObservable()->subscribe(
        [this](const WizardFireball& f) { onWizFireballHit(f); }, mId);
    attachSubToVisibility(mWizFireballSub);
}
void Catalyst::setUpgrades() {
    ParameterSystem::Params<CATALYST> params;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(WIZ_IMGS.at(mId));
    dUp->setEffects(
        {params[CatalystParams::MagicEffect], params[CatalystParams::Range]},
        {}, []() {
            ParameterSystem::Params<CATALYST> params;

            std::stringstream ss;
            ss << "Multiplier: "
               << Upgrade::Defaults::MultiplicativeEffect(
                      params[CatalystParams::MagicEffect].get())
               << "\nRange: " << params[CatalystParams::Range].get();
            return ss.str();
        });
    mMagicEffectDisplay = mUpgrades->subscribe(dUp);

    UpgradePtr up =
        std::make_shared<Upgrade>(params[CatalystParams::RangeUpLvl], 5);
    up->setImage("");
    up->setDescription("Increase range of fireball boost");
    up->setCost(Upgrade::Defaults::CRYSTAL_SHARDS,
                params[CatalystParams::RangeUpCost],
                [](const Number& lvl) { return 5 * (2 ^ lvl); });
    up->setEffect(
        params[CatalystParams::RangeUp],
        [](const Number& lvl) { return 1.1 ^ lvl; },
        Upgrade::Defaults::MultiplicativeEffect);
    mRangeUp = mUpgrades->subscribe(up);
}
void Catalyst::setParamTriggers() {
    ParameterSystem::Params<CATALYST> params;

    mParamSubs.push_back(params[CatalystParams::MagicEffect].subscribeTo(
        {params[CatalystParams::Magic]}, {},
        [this]() { return calcMagicEffect(); }));

    mParamSubs.push_back(params[CatalystParams::Range].subscribeTo(
        {params[CatalystParams::BaseRange], params[CatalystParams::RangeUp]},
        {}, [this]() { return calcRange(); }));

    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[CatalystParams::Magic], params[CatalystParams::Capacity]}, {},
        [this]() { drawMagic(); }));

    mParamSubs.push_back(ParameterSystem::Param(State::BoughtCatalyst)
                             .subscribe([this](bool bought) {
                                 WizardSystem::GetHideObservable()->next(
                                     mId, !bought);
                             }));

    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[CatalystParams::Range]}, {}, [this]() { updateRange(); }));
}

void Catalyst::onWizFireballHit(const WizardFireball& fireball) {
    ParameterSystem::Params<CATALYST> params;
    auto magic = params[CatalystParams::Magic];
    magic.set(max(0, min(magic.get() + fireball.power(),
                         params[CatalystParams::Capacity].get())));
}

void Catalyst::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    TextureBuilder tex;

    tex.draw(mRange);

    tex.draw(mMagicRender);
}

Number Catalyst::calcMagicEffect() {
    ParameterSystem::Params<CATALYST> params;
    return (params[CatalystParams::Magic].get() + 1).logTen() + 1;
}

Number Catalyst::calcRange() {
    ParameterSystem::Params<CATALYST> params;
    return params[CatalystParams::BaseRange].get() *
           params[CatalystParams::RangeUp].get();
}

void Catalyst::drawMagic() {
    ParameterSystem::Params<CATALYST> params;
    mMagicText.text = params[CatalystParams::Magic].get().toString() + "/" +
                      params[CatalystParams::Capacity].get().toString();
    mMagicRender.set(mMagicText);
}

void Catalyst::updateRange() {
    float half = fmaxf(mPos->rect.halfH(), mPos->rect.halfW());
    auto range = ParameterSystem::Param<CATALYST>(CatalystParams::Range);
    mRange.setRadius((int)(half * range.get().toFloat()), ceilf(half / 100),
                     true);
    CatalystRing::GetHitObservable()->setPos(mRange.get());
}

void Catalyst::setPos(float x, float y) {
    WizardBase::setPos(x, y);

    mMagicRender.setDest(
        Rect(mPos->rect.x(), mPos->rect.y2(), mPos->rect.w(), FONT.h));

    mRange.setCenter({mPos->rect.CX(), mPos->rect.CY()});
    CatalystRing::GetHitObservable()->setPos(mRange.get());
}
