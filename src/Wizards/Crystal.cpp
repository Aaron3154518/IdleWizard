#include "Crystal.h"

// Crystal
const Number Crystal::T1_COST1 = 500, Crystal::T1_COST2 = 5000;
const SDL_Color Crystal::MSG_COLOR{225, 0, 200, 255};

Crystal::Crystal() : WizardBase(CRYSTAL) {
    ParameterSystem::ParamList<CRYSTAL> params;
    params.Set(CrystalParams::Magic, 0);
    params.Set(CrystalParams::T1WizardCost, T1_COST1);

    mMsgTData.font = AssetManager::getFont(FONT);
    mMsgTData.color = MSG_COLOR;
}

void Crystal::init() {
    WizardBase::init();

    mMagicText.tData.font = AssetManager::getFont(FONT);

    mUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            std::bind(&Crystal::onUpdate, this, std::placeholders::_1));
    mFireballSub =
        ServiceSystem::Get<FireballService, Fireball::HitObservable>()
            ->subscribe(
                std::bind(&Crystal::onFireballHit, this, std::placeholders::_1),
                mId);
    attachSubToVisibility(mUpdateSub);
    attachSubToVisibility(mFireballSub);

    // Power Display
    UpgradePtr up = std::make_shared<Upgrade>();
    up->setMaxLevel(-1)
        .setEffectSource(
            ParameterSystem::Param<CRYSTAL>(CrystalParams::MagicEffect),
            Upgrade::Defaults::MultiplicativeEffect<CRYSTAL,
                                                    CrystalParams::MagicEffect>)
        .setImg(WIZ_IMGS.at(mId))
        .setDescription("Multiplier based on crystal damage");
    mMagicEffectDisplay = mUpgrades->subscribe(
        [this](UpgradePtr u) {
            ParameterSystem::Param<CRYSTAL> param(CrystalParams::Magic);
            param.set(param.get() * 2);
        },
        up);

    up = std::make_shared<Upgrade>();
    up->setMaxLevel(1)
        .setCostSource(
            ParameterSystem::Param<CRYSTAL>(CrystalParams::T1WizardCost))
        .setMoneySource(Upgrade::Defaults::CRYSTAL_MAGIC)
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
        .setCostSource(
            ParameterSystem::Param<CRYSTAL>(CrystalParams::T1WizardCost))
        .setMoneySource(Upgrade::Defaults::CRYSTAL_MAGIC)
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

    mParamSubs.push_back(
        ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic)
            .subscribe(std::bind(&Crystal::calcMagicEffect, this)));
    mParamSubs.push_back(ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic)
                             .subscribe(std::bind(&Crystal::drawMagic, this)));
}

void Crystal::onUpdate(Time dt) {
    for (auto it = mMessages.begin(), end = mMessages.end(); it != end; ++it) {
        it->mTimer -= dt.ms();
        if (it->mTimer <= 0) {
            if (it->mMoving) {
                it->mTimer = rDist(gen) * 1000 + 750;
                it->mMoving = false;
            } else {
                it = mMessages.erase(it);
                if (it == end) {
                    break;
                }
            }
        } else if (it->mMoving) {
            it->mRData.dest.move(it->mTrajectory.x * dt.s(),
                                 it->mTrajectory.y * dt.s());
        }
    }
}

void Crystal::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    TextureBuilder tex;

    mMagicText.dest = Rect(mPos->rect.x(), mPos->rect.y(), mPos->rect.w(), 0);
    mMagicText.dest.setHeight(FONT.h, Rect::Align::BOT_RIGHT);
    mMagicText.shrinkToTexture();
    tex.draw(mMagicText);

    for (auto msg : mMessages) {
        tex.draw(msg.mRData);
    }

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
            ParameterSystem::Param<CRYSTAL>(CrystalParams::T1WizardCost)
                .set(T1_COST2);
            break;
    }
}

void Crystal::onFireballHit(const Fireball& fireball) {
    switch (fireball.getSourceId()) {
        case WIZARD: {
            ParameterSystem::Param<CRYSTAL> param(CrystalParams::Magic);
            Number magic = param.get() + fireball.getValue();
            param.set(magic);
            mMagicText.tData.text = magic.toString();
            mMagicText.renderText();
            addMessage("+" + fireball.getValue().toString());
        } break;
        case POWER_WIZARD: {
            createFireRing(fireball.getValue(PowerWizardParams::Power));
        } break;
    }
}

void Crystal::calcMagicEffect() {
    ParameterSystem::ParamList<CRYSTAL> params;
    Number effect = (params.Get(CrystalParams::Magic) + 1).logTen() + 1;
    params.Set(CrystalParams::MagicEffect, effect);
}

void Crystal::drawMagic() {
    mMagicText.tData.text =
        ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic).get().toString();
    mMagicText.renderText();
}

std::unique_ptr<FireRing>& Crystal::createFireRing(const Number& val) {
    mFireRings.push_back(std::move(ComponentFactory<FireRing>::New(
        SDL_Point{mPos->rect.CX(), mPos->rect.CY()}, val)));
    return mFireRings.back();
}

void Crystal::addMessage(const std::string& msg) {
    mMsgTData.text = msg;

    RenderData rData;
    rData.texture = mMsgTData.renderText();
    rData.fitToTexture();

    float dx = (rDist(gen) - .5) * mPos->rect.halfW(),
          dy = (rDist(gen) - .5) * mPos->rect.halfH();
    rData.dest.setPos(mPos->rect.cX() + dx, mPos->rect.cY() + dy,
                      Rect::Align::CENTER);
    SDL_FPoint trajectory{copysignf((float)rDist(gen), dx),
                          copysignf((float)rDist(gen), dy)};
    if (trajectory.x == 0 && trajectory.y == 0) {
        trajectory.y = 1;
    }
    float mag = sqrt(trajectory.x * trajectory.x + trajectory.y * trajectory.y);
    float maxSpeed =
        sqrt(mPos->rect.w() * mPos->rect.w() + mPos->rect.h() * mPos->rect.h());
    trajectory.x *= maxSpeed / mag;
    trajectory.y *= maxSpeed / mag;

    mMessages.push_back(
        Message{rData, (int)(rDist(gen) * 250) + 250, true, trajectory});
}
