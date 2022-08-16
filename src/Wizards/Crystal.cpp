#include "Crystal.h"

// Crystal
const Number Crystal::T1Cost1 = 500, Crystal::T1Cost2 = 5000;

Crystal::Crystal() : WizardBase(CRYSTAL) {
    auto params = ParameterSystem::Get();
    params->set<CRYSTAL>(CrystalParams::Magic, 0);
    params->set<CRYSTAL>(CrystalParams::T1WizardCost, T1Cost1);
}

void Crystal::init() {
    WizardBase::init();

    mMagicText.tData.font = AssetManager::getFont(FONT);

    mFireballSub =
        ServiceSystem::Get<FireballService, Fireball::HitObservable>()
            ->subscribe(
                std::bind(&Crystal::onFireballHit, this, std::placeholders::_1),
                mId);
    attachSubToVisibility(mFireballSub);

    // Power Display
    UpgradePtr up = std::make_shared<Upgrade>();
    up->setMaxLevel(-1)
        .setEffectSource<CRYSTAL, CrystalParams::MagicEffect>(
            Upgrade::Defaults::MultiplicativeEffect)
        .setImg(WIZ_IMGS.at(mId))
        .setDescription("Multiplier based on crystal damage");
    mMagicEffectDisplay = mUpgrades->subscribe(
        [this](UpgradePtr u) {
            auto params = ParameterSystem::Get();
            params->set<CRYSTAL>(
                CrystalParams::Magic,
                params->get<CRYSTAL>(CrystalParams::Magic) * 2);
        },
        up);

    up = std::make_shared<Upgrade>();
    up->setMaxLevel(1)
        .setCostSource<CRYSTAL>(CrystalParams::T1WizardCost)
        .setMoneySource<CRYSTAL>(CrystalParams::Magic)
        .setImg(WIZ_IMGS.at(POWER_WIZARD))
        .setDescription(
            "Power Wizard empowers the Wizard and overloads the Crystal for "
            "increased Fireball power");
    mPowWizBuy = mUpgrades->subscribe(
        [this](UpgradePtr u) {
            if (u->getLevel() == 1) {
                mPowWizBuy.reset();
                WizardSystem::GetHideObservable()->next(POWER_WIZARD, false);
                WizardSystem::FireWizEvent(
                    WizardSystem::Event::BoughtPowerWizard);
                WizardSystem::FireWizEvent(
                    mTimeWizBuy ? WizardSystem::Event::BoughtFirstT1
                                : WizardSystem::Event::BoughtSecondT1);
            }
        },
        up);

    up = std::make_shared<Upgrade>();
    up->setMaxLevel(1)
        .setCostSource<CRYSTAL>(CrystalParams::T1WizardCost)
        .setMoneySource<CRYSTAL>(CrystalParams::Magic)
        .setImg(WIZ_IMGS.at(TIME_WIZARD))
        .setDescription(
            "Time Wizard boosts Wizard fire rate and freezes time for a "
            "massive power boost");
    mTimeWizBuy = mUpgrades->subscribe(
        [this](UpgradePtr u) {
            if (u->getLevel() == 1) {
                mTimeWizBuy.reset();
                WizardSystem::GetHideObservable()->next(TIME_WIZARD, false);
                WizardSystem::FireWizEvent(
                    WizardSystem::Event::BoughtTimeWizard);
                WizardSystem::FireWizEvent(
                    mPowWizBuy ? WizardSystem::Event::BoughtFirstT1
                               : WizardSystem::Event::BoughtSecondT1);
            }
        },
        up);

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
}

void Crystal::onHide(WizardId id, bool hide) {
    WizardBase::onHide(id, hide);
    if (hide) {
        switch (id) {
            case CRYSTAL:
                mFireRings.clear();
                break;
        }
    }
}

void Crystal::onWizEvent(WizardSystem::Event e) {
    switch (e) {
        case WizardSystem::Event::BoughtFirstT1:
            ParameterSystem::Get()->set<CRYSTAL>(CrystalParams::T1WizardCost,
                                                 T1Cost2);
            break;
    }
}

void Crystal::onFireballHit(const Fireball& fireball) {
    auto params = ParameterSystem::Get();

    switch (fireball.getSourceId()) {
        case WIZARD: {
            Number magic = params->get<CRYSTAL>(CrystalParams::Magic) +
                           fireball.getValue();
            params->set<CRYSTAL>(CrystalParams::Magic, magic);
            mMagicText.tData.text = magic.toString();
            mMagicText.renderText();
        } break;
        case POWER_WIZARD: {
            createFireRing(fireball.getValue(PowerWizardParams::Power));
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

std::unique_ptr<FireRing>& Crystal::createFireRing(const Number& val) {
    mFireRings.push_back(std::move(ComponentFactory<FireRing>::New(
        SDL_Point{mPos->rect.CX(), mPos->rect.CY()}, val)));
    return mFireRings.back();
}
