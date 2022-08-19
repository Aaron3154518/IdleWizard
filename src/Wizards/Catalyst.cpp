#include "Catalyst.h"

// Catalyst
Catalyst::Catalyst() : WizardBase(CATALYST) {
    ParameterSystem::ParamList<CATALYST> params;
    params.Set(CatalystParams::Magic, 0);
    params.Set(CatalystParams::Capacity, 100);
}

void Catalyst::init() {
    WizardBase::init();

    mMagicText.tData.font = AssetManager::getFont(FONT);

    mFireballSub =
        ServiceSystem::Get<FireballService, Fireball::HitObservable>()
            ->subscribe(std::bind(&Catalyst::onFireballHit, this,
                                  std::placeholders::_1),
                        mId);
    attachSubToVisibility(mFireballSub);

    // Power Display
    UpgradePtr up = std::make_shared<Upgrade>();
    up->setMaxLevel(0)
        .setEffectSource(
            ParameterSystem::Param<CATALYST>(CatalystParams::MagicEffect),
            Upgrade::Defaults::MultiplicativeEffect<
                CATALYST, CatalystParams::MagicEffect>)
        .setImg(WIZ_IMGS.at(mId))
        .setDescription("Multiplier from stored magic");
    mMagicEffectDisplay = mUpgrades->subscribe(up);

    mParamSubs.push_back(
        ParameterSystem::Param<CATALYST>(CatalystParams::Magic)
            .subscribe(std::bind(&Catalyst::calcMagicEffect, this)));
    mParamSubs.push_back(ParameterSystem::ParamList<CATALYST>(
                             {CatalystParams::Magic, CatalystParams::Capacity})
                             .subscribe(std::bind(&Catalyst::drawMagic, this)));
}

void Catalyst::onFireballHit(const Fireball& fireball) {
    switch (fireball.getSourceId()) {
        case WIZARD:
            ParameterSystem::ParamList<CATALYST> params;
            Number magic =
                max(min(params.Get(CatalystParams::Magic) + fireball.getValue(),
                        params.Get(CatalystParams::Capacity)),
                    0);
            params.Set(CatalystParams::Magic, magic);
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
    ParameterSystem::ParamList<CATALYST> params;
    Number effect = (params.Get(CatalystParams::Magic) + 1).logTen() + 1;
    params.Set(CatalystParams::MagicEffect, effect);
}

void Catalyst::drawMagic() {
    ParameterSystem::ParamList<CATALYST> params;
    mMagicText.tData.text = params.Get(CatalystParams::Magic).toString() + "/" +
                            params.Get(CatalystParams::Capacity).toString();
    mMagicText.renderText();
}
