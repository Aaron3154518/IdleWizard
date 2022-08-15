#include "Catalyst.h"

// Catalyst
Catalyst::Catalyst() : WizardBase(CATALYST) {
    auto params = Parameters();
    params->set<CATALYST>(CatalystParams::Magic, 0);
    params->set<CATALYST>(CatalystParams::Capacity, 100);
}

void Catalyst::init() {
    WizardBase::init();

    mMagicText.tData.font = AssetManager::getFont(FONT);

    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&Catalyst::onRender, this, std::placeholders::_1), mPos);
    mTargetSub =
        ServiceSystem::Get<FireballService, TargetObservable>()->subscribe(
            std::bind(&Catalyst::onHit, this, std::placeholders::_1,
                      std::placeholders::_2),
            mId);

    // Power Display
    UpgradePtr up = std::make_shared<Upgrade>();
    up->setMaxLevel(0)
        .setEffectSource<CATALYST, CatalystParams::MagicEffect>(
            Upgrade::Defaults::MultiplicativeEffect)
        .setImg(WIZ_IMGS.at(mId))
        .setDescription("Multiplier from stored magic");
    mMagicEffectDisplay = mUpgrades->subscribe(up);

    auto params = Parameters();
    mParamSubs.push_back(params->subscribe<CATALYST>(
        CatalystParams::Magic, std::bind(&Catalyst::calcMagicEffect, this)));
    mParamSubs.push_back(
        params->subscribe<
            Keys<CATALYST, CatalystParams::Magic, CatalystParams::Capacity>>(
            std::bind(&Catalyst::drawMagic, this)));
}

void Catalyst::onHit(WizardId src, Number val) {
    switch (src) {
        case WIZARD:
            auto params = Parameters();
            Number magic =
                max(min(params->get<CATALYST>(CatalystParams::Magic) + val,
                        params->get<CATALYST>(CatalystParams::Capacity)),
                    0);
            params->set<CATALYST>(CatalystParams::Magic, magic);
            break;
    }
}

void Catalyst::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    mMagicText.dest = Rect(mPos->rect.x(), mPos->rect.y2(), mPos->rect.w(), 0);
    mMagicText.dest.setHeight(FONT.h, Rect::Align::TOP_LEFT);
    mMagicText.fitToTexture();
    TextureBuilder().draw(mMagicText);
}

void Catalyst::calcMagicEffect() {
    auto params = Parameters();
    Number effect =
        (params->get<CATALYST>(CatalystParams::Magic) + 1).logTen() + 1;
    params->set<CATALYST>(CatalystParams::MagicEffect, effect);
}

void Catalyst::drawMagic() {
    mMagicText.tData.text =
        Parameters()->get<CATALYST>(CatalystParams::Magic).toString() + "/" +
        Parameters()->get<CATALYST>(CatalystParams::Capacity).toString();
    mMagicText.renderText();
}
