#include "Catalyst.h"

// Catalyst
Catalyst::Catalyst() : WizardBase(CATALYST) {
    auto params = ParameterSystem::Get();
    params->set<CATALYST>(CatalystParams::Magic, 0);
    params->set<CATALYST>(CatalystParams::Capacity, 100);
}

void Catalyst::init() {
    WizardBase::init();

    mMagicText.tData.font = AssetManager::getFont(FONT);

    mFireballSub =
        ServiceSystem::Get<FireballService, Fireball::HitObservable>()
            ->subscribe(std::bind(&Catalyst::onFireballHit, this,
                                  std::placeholders::_1),
                        mId);

    // Power Display
    UpgradePtr up = std::make_shared<Upgrade>();
    up->setMaxLevel(0)
        .setEffectSource<CATALYST, CatalystParams::MagicEffect>(
            Upgrade::Defaults::MultiplicativeEffect)
        .setImg(WIZ_IMGS.at(mId))
        .setDescription("Multiplier from stored magic");
    mMagicEffectDisplay = mUpgrades->subscribe(up);

    auto params = ParameterSystem::Get();
    mParamSubs.push_back(params->subscribe<CATALYST>(
        CatalystParams::Magic, std::bind(&Catalyst::calcMagicEffect, this)));
    mParamSubs.push_back(
        params->subscribe<
            Keys<CATALYST, CatalystParams::Magic, CatalystParams::Capacity>>(
            std::bind(&Catalyst::drawMagic, this)));
}

void Catalyst::onFireballHit(const Fireball& fireball) {
    switch (fireball.getSourceId()) {
        case WIZARD:
            auto params = ParameterSystem::Get();
            Number magic =
                max(min(params->get<CATALYST>(CatalystParams::Magic) +
                            fireball.getValue(),
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
    auto params = ParameterSystem::Get();
    Number effect =
        (params->get<CATALYST>(CatalystParams::Magic) + 1).logTen() + 1;
    params->set<CATALYST>(CatalystParams::MagicEffect, effect);
}

void Catalyst::drawMagic() {
    mMagicText.tData.text = ParameterSystem::Get()
                                ->get<CATALYST>(CatalystParams::Magic)
                                .toString() +
                            "/" +
                            ParameterSystem::Get()
                                ->get<CATALYST>(CatalystParams::Capacity)
                                .toString();
    mMagicText.renderText();
}
