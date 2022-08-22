#include "Catalyst.h"

// Catalyst
const std::vector<bool> Catalyst::DEFAULT_PARAMS = {
    ParameterSystem::SetDefault<CATALYST>(CatalystParams::Magic, 0),
    ParameterSystem::SetDefault<CATALYST>(CatalystParams::Capacity, 100),
};

Catalyst::Catalyst() : WizardBase(CATALYST) {}

void Catalyst::init() {
    mMagicText.tData.font = AssetManager::getFont(FONT);

    WizardBase::init();
}
void Catalyst::setSubscriptions() {
    mFireballSub =
        ServiceSystem::Get<FireballService, Fireball::HitObservable>()
            ->subscribe(std::bind(&Catalyst::onFireballHit, this,
                                  std::placeholders::_1),
                        mId);
    attachSubToVisibility(mFireballSub);
}
void Catalyst::setUpgrades() {
    // Power Display
    DisplayPtr up = std::make_shared<Display>();
    up->setImage(WIZ_IMGS.at(mId));
    up->setDescription("Multiplier from stored magic");
    up->setEffect(ParameterSystem::Param<CATALYST>(CatalystParams::MagicEffect),
                  Upgrade::Defaults::MultiplicativeEffect);
    mMagicEffectDisplay = mUpgrades->subscribe(up);
}
void Catalyst::setParamTriggers() {
    ParameterSystem::Params<CATALYST> params;
    mParamSubs.push_back(params[CatalystParams::MagicEffect].subscribeTo(
        {params[CatalystParams::Magic]}, {},
        [this]() { return calcMagicEffect(); }));
    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[CatalystParams::Magic], params[CatalystParams::Capacity]}, {},
        [this]() { drawMagic(); }));
    mParamSubs.push_back(ParameterSystem::Param(State::BoughtCatalyst)
                             .subscribe([this](bool bought) {
                                 WizardSystem::GetHideObservable()->next(
                                     mId, !bought);
                             }));
}

void Catalyst::onFireballHit(const Fireball& fireball) {
    switch (fireball.getSourceId()) {
        case WIZARD:
            ParameterSystem::Params<CATALYST> params;
            auto magic = params[CatalystParams::Magic];
            magic.set(max(min(magic.get() + fireball.getValue(),
                              params[CatalystParams::Capacity].get()),
                          0));
            break;
    }
}

void Catalyst::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    mMagicText.dest = Rect(mPos->rect.x(), mPos->rect.y2(), mPos->rect.w(), 0);
    mMagicText.dest.setHeight(FONT.h, Rect::Align::TOP_LEFT);
    mMagicText.shrinkToTexture();
    TextureBuilder().draw(mMagicText);
}

Number Catalyst::calcMagicEffect() {
    ParameterSystem::Params<CATALYST> params;
    return (params[CatalystParams::Magic].get() + 1).logTen() + 1;
}

void Catalyst::drawMagic() {
    ParameterSystem::Params<CATALYST> params;
    mMagicText.tData.text = params[CatalystParams::Magic].get().toString() +
                            "/" +
                            params[CatalystParams::Capacity].get().toString();
    mMagicText.renderText();
}
