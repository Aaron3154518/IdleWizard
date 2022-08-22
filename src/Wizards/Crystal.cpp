#include "Crystal.h"

// Crystal
const Number Crystal::T1_COST1 = 500, Crystal::T1_COST2 = 5e4;
const SDL_Color Crystal::MSG_COLOR{200, 0, 175, 255};

void Crystal::setDefaults() {
    ParameterSystem::Params<CRYSTAL> params;

    // Default 0
    params.setDefaults({CrystalParams::Magic, CrystalParams::Shards}, 0);

    ParameterSystem::States states;

    states[State::ResetT1]->setDefault(false);
}

Crystal::Crystal() : WizardBase(CRYSTAL) {}

void Crystal::init() {
    mMagicText.tData.font = mMsgTData.font = AssetManager::getFont(FONT);
    mMsgTData.color = MSG_COLOR;

    WizardBase::init();
}
void Crystal::setSubscriptions() {
    mUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            [this](Time dt) { onUpdate(dt); });
    mFireballSub =
        ServiceSystem::Get<FireballService, Fireball::HitObservable>()
            ->subscribe([this](const Fireball& f) { onFireballHit(f); }, mId);
    attachSubToVisibility(mUpdateSub);
    attachSubToVisibility(mFireballSub);
}
void Crystal::setUpgrades() {
    ParameterSystem::Params<CRYSTAL> params;
    ParameterSystem::States states;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(WIZ_IMGS.at(mId));
    dUp->setDescription("Multiplier based on crystal damage");
    dUp->setEffect(params[CrystalParams::MagicEffect],
                   Upgrade::Defaults::MultiplicativeEffect);
    mMagicEffectDisplay = mUpgrades->subscribe(dUp);

    // Buy power wizard
    UpgradePtr up =
        std::make_shared<Upgrade>(params[CrystalParams::BuyPowerWizLvl], 1);
    up->setImage(WIZ_IMGS.at(POWER_WIZARD));
    up->setDescription(
        "Power Wizard empowers the Wizard and overloads the Crystal for "
        "increased Fireball power");
    up->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                params[CrystalParams::T1WizardCost]);
    up->setEffect(states[State::BoughtPowerWizard],
                  [](const Number& lvl) { return lvl == 1; });
    mPowWizBuy = mUpgrades->subscribe(up);

    // Buy time wizard
    up = std::make_shared<Upgrade>(params[CrystalParams::BuyTimeWizLvl], 1);
    up->setImage(WIZ_IMGS.at(TIME_WIZARD));
    up->setDescription(
        "Time Wizard boosts Wizard fire rate and freezes time for a "
        "massive power boost");
    up->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                params[CrystalParams::T1WizardCost]);
    up->setEffect(states[State::BoughtTimeWizard],
                  [](const Number& lvl) { return lvl == 1; });
    mTimeWizBuy = mUpgrades->subscribe(up);

    // Buy catalyst
    up = std::make_shared<Upgrade>(params[CrystalParams::BuyCatalystLvl], 1);
    up->setImage(WIZ_IMGS.at(CATALYST));
    up->setDescription(
        "Catalyst stores magic and boosts fireballs that pass "
        "nearby");
    up->setCost(Upgrade::Defaults::CRYSTAL_SHARDS,
                params[CrystalParams::CatalystCost]);
    up->setEffect(states[State::BoughtCatalyst],
                  [](const Number& lvl) { return lvl == 1; });
    mCatalystBuy = mUpgrades->subscribe(up);
}
void Crystal::setParamTriggers() {
    ParameterSystem::Params<CRYSTAL> params;
    ParameterSystem::States states;

    mParamSubs.push_back(params[CrystalParams::MagicEffect].subscribeTo(
        {params[CrystalParams::Magic]}, {},
        [this]() { return calcMagicEffect(); }));

    mParamSubs.push_back(params[CrystalParams::ShardGain].subscribeTo(
        {params[CrystalParams::Magic]}, {},
        [this]() { return calcShardGain(); }));

    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[CrystalParams::Magic], params[CrystalParams::Shards]}, {},
        [this]() { drawMagic(); }));

    mParamSubs.push_back(params[CrystalParams::T1WizardCost].subscribeTo(
        states[State::BoughtFirstT1],
        [this](bool val) { return val ? T1_COST2 : T1_COST1; }));

    mParamSubs.push_back(states[State::BoughtFirstT1].subscribeTo(
        {}, {states[State::BoughtPowerWizard], states[State::BoughtTimeWizard]},
        []() {
            ParameterSystem::States states;
            return states[State::BoughtPowerWizard].get() ||
                   states[State::BoughtTimeWizard].get();
        }));

    mParamSubs.push_back(states[State::BoughtSecondT1].subscribeTo(
        {}, {states[State::BoughtPowerWizard], states[State::BoughtTimeWizard]},
        []() {
            ParameterSystem::States states;
            return states[State::BoughtPowerWizard].get() &&
                   states[State::BoughtTimeWizard].get();
        }));

    mParamSubs.push_back(states[State::ResetT1].subscribe(
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
    if (clicked) {
        auto param = ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic);
        param.set(param.get() * 2);
        if (param.get() > Number(1, 6)) {
            triggerT1Reset();
        }
    }
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

void Crystal::onResetT1() { mFireRings.clear(); }

void Crystal::onFireballHit(const Fireball& fireball) {
    switch (fireball.getSourceId()) {
        case WIZARD: {
            auto magic = ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic);
            magic.set(magic.get() + fireball.getValue());
            addMessage("+" + fireball.getValue().toString());
        } break;
        case POWER_WIZARD: {
            createFireRing(fireball.getValue(PowerWizardParams::Power));
        } break;
    }
}

Number Crystal::calcMagicEffect() {
    ParameterSystem::Params<CRYSTAL> params;
    return (params[CrystalParams::Magic].get() + 1).logTen() + 1;
}

Number Crystal::calcShardGain() {
    ParameterSystem::Params<CRYSTAL> params;
    Number shards = (params[CrystalParams::Magic].get() + 1).logTen();
    if (shards < 1) {
        shards = 0;
    }
    return shards;
}

void Crystal::drawMagic() {
    ParameterSystem::Params<CRYSTAL> params;
    std::stringstream ss;
    ss << params[CrystalParams::Magic].get() << "\n"
       << params[CrystalParams::Shards].get();
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
    // Events::send(Event::T1Reset);
    auto resetT1 = ParameterSystem::Param(State::ResetT1);
    if (!resetT1.get()) {
        resetT1.set(true);
    }
}
