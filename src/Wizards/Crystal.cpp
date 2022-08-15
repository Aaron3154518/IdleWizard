#include "Crystal.h"

// Crystal
Crystal::Crystal() : WizardBase(CRYSTAL) {
    auto params = ParameterSystem::Get();
    params->set<CRYSTAL>(CrystalParams::Magic, 0);
}

void Crystal::init() {
    WizardBase::init();

    mMagicText.tData.font = AssetManager::getFont(FONT);

    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&Crystal::onRender, this, std::placeholders::_1), mPos);
    mMouseSub = ServiceSystem::Get<MouseService, MouseObservable>()->subscribe(
        std::bind(&Crystal::onClick, this, std::placeholders::_1,
                  std::placeholders::_2),
        mPos);
    mTargetSub =
        ServiceSystem::Get<FireballService, TargetObservable>()->subscribe(
            std::bind(&Crystal::onHit, this, std::placeholders::_1,
                      std::placeholders::_2),
            mId);

    // Power Display
    UpgradePtr up = std::make_shared<Upgrade>();
    up->setMaxLevel(0)
        .setEffectSource<CRYSTAL, CrystalParams::MagicEffect>(
            Upgrade::Defaults::MultiplicativeEffect)
        .setImg(WIZ_IMGS.at(mId))
        .setDescription("Multiplier based on crystal damage");
    mMagicEffectDisplay = mUpgrades->subscribe(up);

    auto params = ParameterSystem::Get();
    mParamSubs.push_back(params->subscribe<CRYSTAL>(
        CrystalParams::Magic, std::bind(&Crystal::calcMagicEffect, this)));
    mParamSubs.push_back(params->subscribe<CRYSTAL>(
        CrystalParams::Magic, std::bind(&Crystal::drawMagic, this)));
}

void Crystal::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    mMagicText.dest = Rect(mPos->rect.x(), mPos->rect.y(), mPos->rect.w(), 0);
    mMagicText.dest.setHeight(FONT.h, Rect::Align::BOT_RIGHT);
    mMagicText.fitToTexture();
    TextureBuilder().draw(mMagicText);

    for (auto it = mFireRings.begin(); it != mFireRings.end(); ++it) {
        if ((*it)->dead()) {
            it = mFireRings.erase(it);
            if (it == mFireRings.end()) {
                break;
            }
        }
    }
}

void Crystal::onClick(Event::MouseButton b, bool clicked) {
    WizardBase::onClick(b, clicked);
    if (clicked) {
        auto params = ParameterSystem::Get();
        params->set<CRYSTAL>(CrystalParams::Magic,
                             params->get<CRYSTAL>(CrystalParams::Magic) * 10);
    }
}

void Crystal::onHit(WizardId src, const Number& val) {
    auto params = ParameterSystem::Get();

    switch (src) {
        case WIZARD: {
            Number magic = params->get<CRYSTAL>(CrystalParams::Magic) + val;
            params->set<CRYSTAL>(CrystalParams::Magic, magic);
            mMagicText.tData.text = magic.toString();
            mMagicText.renderText();
        } break;
        case POWER_WIZARD: {
            createFireRing();
        } break;
    }
}

void Crystal::calcMagicEffect() {
    auto params = ParameterSystem::Get();
    Number effect =
        (params->get<CRYSTAL>(CrystalParams::Magic) + 1).logTen() + 1;
    params->set<CRYSTAL>(CrystalParams::MagicEffect, effect);
}

void Crystal::drawMagic() {
    mMagicText.tData.text =
        ParameterSystem::Get()->get<CRYSTAL>(CrystalParams::Magic).toString();
    mMagicText.renderText();
}

std::unique_ptr<FireRing>& Crystal::createFireRing() {
    mFireRings.push_back(std::move(ComponentFactory<FireRing>::New(
        SDL_Point{mPos->rect.CX(), mPos->rect.CY()},
        ParameterSystem::Get()->get<POWER_WIZARD>(
            PowerWizardParams::FireRingEffect))));
    return mFireRings.back();
}
