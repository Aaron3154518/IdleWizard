#include "Crystal.h"

// Crystal
const Number Crystal::T1_COST1 = 500, Crystal::T1_COST2 = 5e4;
const SDL_Color Crystal::MSG_COLOR{200, 0, 175, 255};

const AnimationData Crystal::IMG{"res/wizards/crystal_ss.png", 13, 100};

void Crystal::setDefaults() {
    using WizardSystem::ResetTier;

    ParameterSystem::Params<CRYSTAL> params;

    // Default 0
    params[CrystalParams::Magic]->init(0, ResetTier::T1);
    params[CrystalParams::Shards]->init(0, ResetTier::T2);

    params[CrystalParams::WizardCntUpCost]->init(Number(2, 3));
    params[CrystalParams::CatalystCost]->init(1);

    ParameterSystem::States states;

    states[State::ResetT1]->init(false);
    states[State::BoughtCatWizCntUp]->init(false, ResetTier::T1);
    states[State::BoughtPowerWizard]->init(false, ResetTier::T1);
    states[State::BoughtTimeWizard]->init(false, ResetTier::T1);
    states[State::BoughtCatalyst]->init(false, ResetTier::T2);
}

Crystal::Crystal() : WizardBase(CRYSTAL) {}

void Crystal::init() {
    ParameterSystem::Params<CRYSTAL> params;

    mImg.set(IMG).setDest(IMG_RECT);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);

    SDL_Point screenDim = RenderSystem::getWindowSize();
    setPos(screenDim.x / 2, screenDim.y / 2);

    mMagicText->setFont(FONT).setImgs(
        {Money::GetMoneyIcon(params[CrystalParams::Magic]),
         Money::GetMoneyIcon(params[CrystalParams::Shards])});
    mMagicRender.set(mMagicText)
        .setFit(RenderData::FitMode::Texture)
        .setFitAlign(Rect::Align::CENTER, Rect::Align::TOP_LEFT);
    mMsgTData.setFont(FONT).setColor(MSG_COLOR);

    WizardBase::init();
}
void Crystal::setSubscriptions() {
    mUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            [this](Time dt) { onUpdate(dt); });
    mWizFireballHitSub = WizardFireball::GetHitObservable()->subscribe(
        [this](const WizardFireball& f) { onWizFireballHit(f); }, mId);
    mPowFireballHitSub = PowerWizFireball::GetHitObservable()->subscribe(
        [this](const PowerWizFireball& f) { onPowFireballHit(f); }, mId);
    mAnimTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            mImg.nextFrame();
            WizardSystem::GetWizardImageObservable()->next(mId, mImg);
            t.length =
                mImg.getFrame() == 0 ? getAnimationDelay() : IMG.frame_ms;
            return true;
        },
        IMG);
    attachSubToVisibility(mUpdateSub);
    attachSubToVisibility(mWizFireballHitSub);
    attachSubToVisibility(mPowFireballHitSub);
}
void Crystal::setUpgrades() {
    ParameterSystem::Params<CRYSTAL> params;
    ParameterSystem::States states;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(mId);
    dUp->setDescription({"Multiplier based on crystal damage"});
    dUp->setEffect(params[CrystalParams::MagicEffect],
                   Upgrade::Defaults::MultiplicativeEffect);
    mMagicEffectDisplay = mUpgrades->subscribe(dUp);

    // Wizard count upgrade
    BuyablePtr bUp =
        std::make_shared<Buyable>(states[State::BoughtCatWizCntUp]);
    bUp->setImage("");
    bUp->setDescription(
        {"Wizards synergy provides a multiplier based on the number of active "
         "wizards"});
    bUp->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                 params[CrystalParams::WizardCntUpCost]);
    bUp->setEffect(params[CrystalParams::WizardCntEffect],
                   Upgrade::Defaults::MultiplicativeEffect);
    mWizCntUp = mUpgrades->subscribe(bUp);

    // Buy power wizard
    bUp = std::make_shared<Buyable>(states[State::BoughtPowerWizard]);
    bUp->setImage(WIZ_IMGS.at(POWER_WIZARD));
    bUp->setDescription(
        {"Power Wizard empowers the Wizard and overloads the Crystal for "
         "increased Fireball power"});
    bUp->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                 params[CrystalParams::T1WizardCost]);
    mPowWizBuy = mUpgrades->subscribe(bUp);

    // Buy time wizard
    bUp = std::make_shared<Buyable>(states[State::BoughtTimeWizard]);
    bUp->setImage(WIZ_IMGS.at(TIME_WIZARD));
    bUp->setDescription(
        {"Time Wizard boosts Wizard fire rate and freezes time for a "
         "massive power boost"});
    bUp->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                 params[CrystalParams::T1WizardCost]);
    mTimeWizBuy = mUpgrades->subscribe(bUp);

    // Buy catalyst
    bUp = std::make_shared<Buyable>(states[State::BoughtCatalyst]);
    bUp->setImage(WIZ_IMGS.at(CATALYST));
    bUp->setDescription(
        {"Catalyst stores magic and boosts fireballs that pass "
         "nearby"});
    bUp->setCost(Upgrade::Defaults::CRYSTAL_SHARDS,
                 params[CrystalParams::CatalystCost]);
    mCatalystBuy = mUpgrades->subscribe(bUp);
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

    mParamSubs.push_back(params[CrystalParams::NumWizards].subscribeTo(
        {},
        {states[State::BoughtPowerWizard], states[State::BoughtTimeWizard],
         states[State::BoughtCatalyst]},
        [this]() { return calcNumWizards(); }));

    mParamSubs.push_back(params[CrystalParams::WizardCntEffect].subscribeTo(
        {params[CrystalParams::NumWizards]}, {states[State::BoughtCatWizCntUp]},
        [this]() { return calcWizCntEffect(); }));

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

    mParamSubs.push_back(states[State::BoughtFirstT1].subscribe(
        [this](bool val) { mWizCntUp->setActive(val); }));

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
            Rect msgR = it->mRData.getRect();
            msgR.move(it->mTrajectory.x * dt.s(), it->mTrajectory.y * dt.s());
            it->mRData.setDest(msgR);
        }
    }
}

void Crystal::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    TextureBuilder tex;

    tex.draw(mMagicRender);

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

bool addMagic = false;
void Crystal::onClick(Event::MouseButton b, bool clicked) {
    if (addMagic && clicked) {
        auto param = ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic);
        param.set(param.get() * 3);
        if (param.get() > Number(1, 6)) {
            // triggerT1Reset();
        }
    }
    addMagic = clicked;
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

void Crystal::onReset(WizardSystem::ResetTier tier) { mFireRings.clear(); }

void Crystal::onWizFireballHit(const WizardFireball& fireball) {
    auto magic = ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic);
    magic.set(magic.get() + fireball.power());
    addMessage("+" + fireball.power().toString());
}

void Crystal::onPowFireballHit(const PowerWizFireball& fireball) {
    createFireRing(fireball.power());
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

Number Crystal::calcNumWizards() {
    ParameterSystem::States states;

    // Wizard is always unlocked
    // Convert state variables to ints to get count
    return 1 + states[State::BoughtPowerWizard].get() +
           states[State::BoughtTimeWizard].get() +
           states[State::BoughtCatalyst].get();
}

Number Crystal::calcWizCntEffect() {
    if (!ParameterSystem::Param(State::BoughtCatWizCntUp).get()) {
        return 1;
    }

    return ParameterSystem::Param<CRYSTAL>(CrystalParams::NumWizards).get() ^ 2;
}

void Crystal::drawMagic() {
    ParameterSystem::Params<CRYSTAL> params;
    std::stringstream ss;
    ss << "{i}" << params[CrystalParams::Magic].get();
    if (ParameterSystem::Param(State::ResetT1).get()) {
        ss << "\n{i}" << params[CrystalParams::Shards].get();
    }
    mMagicText->setText(ss.str(), mPos->rect.W());
}

int Crystal::getAnimationDelay() {
    auto magic = ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic);
    return fmaxf(0, 10000 - ((magic.get() + 1).logTen() ^ 2).toFloat());
}

std::unique_ptr<FireRing>& Crystal::createFireRing(const Number& val) {
    mFireRings.push_back(std::move(ComponentFactory<FireRing>::New(
        SDL_Point{mPos->rect.CX(), mPos->rect.CY()}, val)));
    return mFireRings.back();
}

void Crystal::addMessage(const std::string& msg) {
    mMsgTData.setText(msg);

    float dx = (rDist(gen) - .5) * mPos->rect.halfW(),
          dy = (rDist(gen) - .5) * mPos->rect.halfH();
    RenderData rData =
        RenderData()
            .set(mMsgTData)
            .setFit(RenderData::FitMode::Texture)
            .setDest(Rect(mPos->rect.cX() + dx, mPos->rect.cY() + dy, 0, 0));

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
    auto shards = ParameterSystem::Param<CRYSTAL>(CrystalParams::Shards);
    auto shardGain = ParameterSystem::Param<CRYSTAL>(CrystalParams::ShardGain);
    shards.set(shards.get() + shardGain.get());

    WizardSystem::Reset(WizardSystem::ResetTier::T1);

    auto resetT1 = ParameterSystem::Param(State::ResetT1);
    if (!resetT1.get()) {
        resetT1.set(true);
    }
}

void Crystal::setPos(float x, float y) {
    WizardBase::setPos(x, y);

    mMagicRender.setDest(
        Rect(mPos->rect.x(), mPos->rect.y2(), mPos->rect.w(), FONT.h * 2));
}
