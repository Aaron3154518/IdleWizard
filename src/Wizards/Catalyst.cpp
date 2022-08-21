#include "Catalyst.h"

// Catalyst
Catalyst::Catalyst() : WizardBase(CATALYST) {}

void Catalyst::init() {
    mMagicText.tData.font = AssetManager::getFont(FONT);

    WizardBase::init();
}
void Catalyst::setDefaultValues() {
    ParameterSystem::Params<CATALYST> params;
    params.set(CatalystParams::Magic, 0);
    params.set(CatalystParams::Capacity, 100);
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
    UpgradePtr up = std::make_shared<Upgrade>();
    up->setMaxLevel(0)
        .setEffectSource(
            ParameterSystem::Param<CATALYST>(CatalystParams::MagicEffect),
            Upgrade::Defaults::MultiplicativeEffect)
        .setImg(WIZ_IMGS.at(mId))
        .setDescription("Multiplier from stored magic");
    mMagicEffectDisplay = mUpgrades->subscribe(up);
}
void Catalyst::setParamTriggers() {
    mParamSubs.push_back(
        ParameterSystem::Param<CATALYST>(CatalystParams::Magic)
            .subscribe(std::bind(&Catalyst::calcMagicEffect, this)));
    mParamSubs.push_back(ParameterSystem::ParamMap<CATALYST>(
                             {CatalystParams::Magic, CatalystParams::Capacity})
                             .subscribe(std::bind(&Catalyst::drawMagic, this)));
}
void Catalyst::setEventTriggers() {
    WizardSystem::Events events;
    mStateSubs.push_back(
        events.subscribe(WizardSystem::BoughtCatalyst, [this](bool bought) {
            WizardSystem::GetHideObservable()->next(mId, !bought);
        }));
}

void Catalyst::onFireballHit(const Fireball& fireball) {
    switch (fireball.getSourceId()) {
        case WIZARD:
            ParameterSystem::Params<CATALYST> params;
            Number magic =
                max(min(params.get(CatalystParams::Magic) + fireball.getValue(),
                        params.get(CatalystParams::Capacity)),
                    0);
            params.set(CatalystParams::Magic, magic);
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

void Catalyst::calcMagicEffect() {
    ParameterSystem::Params<CATALYST> params;
    Number effect = (params.get(CatalystParams::Magic) + 1).logTen() + 1;
    params.set(CatalystParams::MagicEffect, effect);
}

void Catalyst::drawMagic() {
    ParameterSystem::Params<CATALYST> params;
    mMagicText.tData.text = params.get(CatalystParams::Magic).toString() + "/" +
                            params.get(CatalystParams::Capacity).toString();
    mMagicText.renderText();
}
