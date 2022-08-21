#include "Crystal.h"

// Crystal
const Number Crystal::T1_COST1 = 500, Crystal::T1_COST2 = 5e4;
const SDL_Color Crystal::MSG_COLOR{200, 0, 175, 255};

Crystal::Crystal() : WizardBase(CRYSTAL) {}

void Crystal::init() {
    mMagicText.tData.font = mMsgTData.font = AssetManager::getFont(FONT);
    mMsgTData.color = MSG_COLOR;

    WizardBase::init();
}
void Crystal::setDefaultValues() {
    ParameterSystem::Params<CRYSTAL> params;
    params.set(CrystalParams::Magic, 0);
    params.set(CrystalParams::Shards, 0);
    params.set(CrystalParams::CatalystCost, 1);
}
void Crystal::setSubscriptions() {
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
}
void Crystal::setUpgrades() {
    // Power Display
    UpgradePtr up = std::make_shared<Upgrade>();
    up->setMaxLevel(-1)
        .setEffectSource(
            ParameterSystem::Param<CRYSTAL>(CrystalParams::MagicEffect),
            Upgrade::Defaults::MultiplicativeEffect)
        .setImg(WIZ_IMGS.at(mId))
        .setDescription("Multiplier based on crystal damage");
    mMagicEffectDisplay = mUpgrades->subscribe(
        [this](UpgradePtr u) {
            ParameterSystem::Param<CRYSTAL> param(CrystalParams::Magic);
            param.set(param.get() * 2);
            if (param.get() > Number(1, 6)) {
                triggerT1Reset();
            }
        },
        up);

    // Buy power wizard
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
            WizardSystem::States::set(WizardSystem::State::BoughtPowerWizard,
                                      u->getLevel() == 1);
        },
        up);

    // Buy time wizard
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
            WizardSystem::States::set(WizardSystem::State::BoughtTimeWizard,
                                      u->getLevel() == 1);
        },
        up);

    // Buy catalyst
    up = std::make_shared<Upgrade>();
    up->setMaxLevel(1)
        .setCostSource(
            ParameterSystem::Param<CRYSTAL>(CrystalParams::CatalystCost))
        .setMoneySource(Upgrade::Defaults::CRYSTAL_SHARDS)
        .setImg(WIZ_IMGS.at(CATALYST))
        .setDescription(
            "Catalyst stores magic and uses it to boost fireballs that pass "
            "nearby");
    mCatalystBuy = mUpgrades->subscribe(
        [this](UpgradePtr u) {
            WizardSystem::States::set(WizardSystem::State::BoughtCatalyst,
                                      u->getLevel() == 1);
        },
        up);
}
void Crystal::setParamTriggers() {
    mParamSubs.push_back(
        ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic)
            .subscribe(std::bind(&Crystal::calcMagicEffect, this)));
    mParamSubs.push_back(
        ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic)
            .subscribe(std::bind(&Crystal::calcShardGain, this)));
    mParamSubs.push_back(ParameterSystem::ParamMap<CRYSTAL>(
                             {CrystalParams::Magic, CrystalParams::Shards})
                             .subscribe(std::bind(&Crystal::drawMagic, this)));
}
void Crystal::setEventTriggers() {
    WizardSystem::States states;
    mStateSubs.push_back(
        states.subscribe(WizardSystem::State::BoughtFirstT1, [this](bool val) {
            ParameterSystem::Param<CRYSTAL>(CrystalParams::T1WizardCost)
                .set(val ? T1_COST2 : T1_COST1);
        }));
    mStateSubs.push_back(states.subscribe(
        {WizardSystem::State::BoughtPowerWizard,
         WizardSystem::State::BoughtTimeWizard},
        []() {
            WizardSystem::States states;
            bool power = states.get(WizardSystem::State::BoughtPowerWizard),
                 time = states.get(WizardSystem::State::BoughtTimeWizard);
            states.set(WizardSystem::State::BoughtFirstT1, power || time);
            states.set(WizardSystem::State::BoughtSecondT1, power && time);
        }));
    mStateSubs.push_back(
        states.subscribe(WizardSystem::State::ResetT1,
                         [this](bool val) { mCatalystBuy->setActive(val); }));
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

    mMagicText.dest =
        Rect(mPos->rect.x(), mPos->rect.y2(), mPos->rect.w(), FONT.h * 2);
    mMagicText.shrinkToTexture(Rect::CENTER, Rect::TOP_LEFT);
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

void Crystal::onResetT1() {
    ParameterSystem::Params<CRYSTAL> params;
    Number shards = params.get(CrystalParams::Shards) +
                    params.get(CrystalParams::ShardGain);
    int catLvl = 0;
    if (mCatalystBuy) {
        catLvl = UpgradeList::Get(mCatalystBuy)->getLevel();
    }

    WizardBase::onResetT1();

    WizardSystem::States states;
    states.set(WizardSystem::State::BoughtPowerWizard, false);
    states.set(WizardSystem::State::BoughtTimeWizard, false);

    mFireRings.clear();

    params.set(CrystalParams::Shards, shards);
    if (mCatalystBuy) {
        auto up = UpgradeList::Get(mCatalystBuy);
        up->setLevel(catLvl);
        mCatalystBuy->get<UpgradeList::ON_LEVEL>()(up);
        up->updateInfo();
    }
}

void Crystal::onFireballHit(const Fireball& fireball) {
    switch (fireball.getSourceId()) {
        case WIZARD: {
            ParameterSystem::Param<CRYSTAL> param(CrystalParams::Magic);
            Number magic = param.get() + fireball.getValue();
            param.set(magic);
            addMessage("+" + fireball.getValue().toString());
        } break;
        case POWER_WIZARD: {
            createFireRing(fireball.getValue(PowerWizardParams::Power));
        } break;
    }
}

void Crystal::calcMagicEffect() {
    ParameterSystem::Params<CRYSTAL> params;
    Number effect = (params.get(CrystalParams::Magic) + 1).logTen() + 1;
    params.set(CrystalParams::MagicEffect, effect);
}

void Crystal::calcShardGain() {
    ParameterSystem::Params<CRYSTAL> params;
    Number shards = (params.get(CrystalParams::Magic) + 1).logTen();
    if (shards < 1) {
        shards = 0;
    }
    params.set(CrystalParams::ShardGain, shards);
}

void Crystal::drawMagic() {
    ParameterSystem::Params<CRYSTAL> params;
    std::stringstream ss;
    ss << params.get(CrystalParams::Magic) << "\n"
       << params.get(CrystalParams::Shards);
    mMagicText.tData.text = ss.str();
    mMagicText.tData.w = mPos->rect.W();
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
    SDL_FPoint trajectory{copysignf(rDist(gen), dx), copysignf(rDist(gen), dy)};
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

void Crystal::triggerT1Reset() {
    WizardSystem::Events::send(WizardSystem::Event::T1Reset);
    WizardSystem::States state;
    if (!state.get(WizardSystem::State::ResetT1)) {
        state.set(WizardSystem::State::ResetT1, true);
    }
}
