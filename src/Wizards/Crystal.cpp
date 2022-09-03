#include "Crystal.h"

// Crystal
const Number Crystal::T1_COST1 = 500, Crystal::T1_COST2 = 5e4;
const SDL_Color Crystal::MSG_COLOR{200, 0, 175, 255};

const AnimationData Crystal::IMG{"res/wizards/crystal_ss.png", 13, 100};

void Crystal::setDefaults() {
    using WizardSystem::Event;

    ParameterSystem::Params<CRYSTAL> params;

    // Default 0
    params[CrystalParams::Magic]->init(Number(1, 10), Event::ResetT1);
    params[CrystalParams::Shards]->init(0, Event::ResetT2);

    params[CrystalParams::WizardCntUpCost]->init(Number(2, 3));
    params[CrystalParams::GlowUpCost]->init(Number(1, 5));
    params[CrystalParams::CatalystCost]->init(1);

    ParameterSystem::States states;

    states[State::ResetT1]->init(false);
    states[State::BoughtCrysWizCntUp]->init(false, Event::ResetT1);
    states[State::BoughtCrysGlowUp]->init(false, Event::ResetT1);
    states[State::BoughtPowerWizard]->init(false, Event::ResetT1);
    states[State::BoughtTimeWizard]->init(false, Event::ResetT1);
    states[State::BoughtCatalyst]->init(false, Event::ResetT2);
}

RenderDataWPtr Crystal::GetIcon() {
    static RenderDataPtr ICON;
    static TimerObservable::SubscriptionPtr ANIM_SUB;
    if (!ICON) {
        ICON = std::make_shared<RenderData>();
        ICON->set(IMG);
        ANIM_SUB =
            ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
                [](Timer& t) {
                    ICON->nextFrame();
                    return true;
                },
                Timer(IMG.frame_ms));
    }

    return ICON;
}

Crystal::Crystal() : WizardBase(CRYSTAL) {}

void Crystal::init() {
    ParameterSystem::Params<CRYSTAL> params;

    mImg.set(IMG).setDest(IMG_RECT);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);

    mMagicText->setFont(FONT).setImgs(
        {Money::GetMoneyIcon(params[CrystalParams::Magic]),
         Money::GetMoneyIcon(params[CrystalParams::Shards])});
    mMagicRender.set(mMagicText)
        .setFit(RenderData::FitMode::Texture)
        .setFitAlign(Rect::Align::CENTER, Rect::Align::TOP_LEFT);
    mMsgTData.setFont(FONT).setColor(MSG_COLOR);

    WizardBase::init();

    SDL_Point screenDim = RenderSystem::getWindowSize();
    setPos(screenDim.x / 2, screenDim.y / 2);
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
    mT1ResetSub = WizardSystem::GetWizardEventObservable()->subscribe(
        [this]() { onT1Reset(); }, WizardSystem::Event::ResetT1);
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
        std::make_shared<Buyable>(states[State::BoughtCrysWizCntUp]);
    bUp->setImage("");
    bUp->setDescription(
        {"Wizards synergy provides a multiplier based on the number of active "
         "wizards"});
    bUp->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                 params[CrystalParams::WizardCntUpCost]);
    bUp->setEffect(params[CrystalParams::WizardCntEffect],
                   Upgrade::Defaults::MultiplicativeEffect);
    mWizCntUp = mUpgrades->subscribe(bUp);

    // Glow upgrade
    bUp = std::make_shared<Buyable>(states[State::BoughtCrysGlowUp]);
    bUp->setImage("");
    bUp->setDescription(
        {"After begin struck by the power wizard, the crystal will absorb "
         "fireball strikes and multiply their power"});
    bUp->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                 params[CrystalParams::GlowUpCost]);
    bUp->setEffect(params[CrystalParams::GlowEffect],
                   Upgrade::Defaults::MultiplicativeEffect);
    mGlowUp = mUpgrades->subscribe(bUp);

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
        {params[CrystalParams::NumWizards]}, {states[State::BoughtCrysGlowUp]},
        [this]() { return calcWizCntEffect(); }));

    mParamSubs.push_back(params[CrystalParams::GlowEffect].subscribeTo(
        {params[CrystalParams::WizardCntEffect]},
        {states[State::BoughtCrysGlowUp]},
        [this]() { return calcGlowEffect(); }));

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

    mParamSubs.push_back(ParameterSystem::subscribe(
        {}, {states[State::BoughtSecondT1], states[State::BoughtCrysWizCntUp]},
        [this]() {
            ParameterSystem::States states;
            mGlowUp->setActive(states[State::BoughtSecondT1].get() &&
                               states[State::BoughtCrysWizCntUp].get());
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
        if (false && param.get() > Number(1, 6)) {
            triggerT1Reset();
        }
    }
    addMagic = clicked;
    WizardBase::onClick(b, clicked);
}

void Crystal::onHide(bool hide) {
    WizardBase::onHide(hide);
    if (hide) {
        mFireRings.clear();
    }
}

void Crystal::onT1Reset() { mFireRings.clear(); }

void Crystal::onWizFireballHit(const WizardFireball& fireball) {
    auto magic = ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic);
    magic.set(magic.get() + fireball.getPower());
    addMessage("+" + fireball.getPower().toString());
    if (ParameterSystem::Param(State::BoughtCrysGlowUp).get()) {
        mGlowMagic += fireball.getPower();
    }
}

void Crystal::onPowFireballHit(const PowerWizFireball& fireball) {
    createFireRing(fireball.getPower());
    if (ParameterSystem::Param(State::BoughtCrysGlowUp).get()) {
        mGlowMagic = 0;
        mGlowTimerSub = TimeSystem::GetTimerObservable()->subscribe(
            [this](Timer& t) { return onGlowTimer(t); },
            Timer(fireball.getDuration().toFloat()));
    }
}

bool Crystal::onGlowTimer(Timer& t) {
    ParameterSystem::Params<CRYSTAL> params;
    Number magic = mGlowMagic * params[CrystalParams::GlowEffect].get();
    params[CrystalParams::Magic].set(params[CrystalParams::Magic].get() +
                                     magic);
    addMessage("+" + magic.toString());
    return false;
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
    if (!ParameterSystem::Param(State::BoughtCrysWizCntUp).get()) {
        return 1;
    }

    return ParameterSystem::Param<CRYSTAL>(CrystalParams::NumWizards).get() ^ 2;
}

Number Crystal::calcGlowEffect() {
    ParameterSystem::Params<CRYSTAL> params;
    ParameterSystem::States states;

    if (!states[State::BoughtCrysGlowUp].get()) {
        return 1;
    }

    return params[CrystalParams::WizardCntEffect].get() ^ 1.5;
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

    WizardSystem::GetWizardEventObservable()->next(
        WizardSystem::Event::ResetT1);

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
