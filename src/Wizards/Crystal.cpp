#include "Crystal.h"

// Crystal
Crystal::Crystal() : WizardBase(CRYSTAL) {}

void Crystal::init() {
    ParameterSystem::Params<CRYSTAL> params;

    mImg.set(CrystalDefs::IMG).setDest(IMG_RECT);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);
    mGlowBkgrnd.set(CrystalDefs::GLOW_EFFECT_IMG);
    SDL_Point imgDim = mImg.getTextureDim(),
              glowDim = mGlowBkgrnd.getTextureDim();
    mGlowBkgrnd.setDest(Rect(0, 0, mPos->rect.w() * glowDim.x / imgDim.x,
                             mPos->rect.h() * glowDim.y / imgDim.y));

    mMagicText->setFont(FONT).setImgs(
        {Money::GetMoneyIcon(params[CrystalParams::Magic]),
         Money::GetMoneyIcon(params[CrystalParams::Shards])});
    mMagicRender.set(mMagicText)
        .setFit(RenderData::FitMode::Texture)
        .setFitAlign(Rect::Align::CENTER, Rect::Align::TOP_LEFT);
    mMsgTData.setFont(FONT);

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
            t.length = mImg.getFrame() == 0 ? getAnimationDelay()
                                            : CrystalDefs::IMG.frame_ms;
            return true;
        },
        CrystalDefs::IMG);
    mGlowAnimTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            mGlowBkgrnd.nextFrame();
            if (mGlowBkgrnd.getFrame() == 0) {
                mGlowAnimTimerSub->setActive(false);
            }
            return true;
        },
        CrystalDefs::GLOW_EFFECT_IMG);
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
    dUp->setDescription(
        {"Multiplier based on {i}",
         {Money::GetMoneyIcon(Upgrade::Defaults::CRYSTAL_MAGIC)}});
    dUp->setEffect(params[CrystalParams::MagicEffect],
                   Upgrade::Defaults::MultiplicativeEffect);
    mMagicEffectDisplay = mUpgrades->subscribe(dUp);

    // Wizard count upgrade
    BuyablePtr bUp =
        std::make_shared<Buyable>(states[State::BoughtCrysWizCntUp]);
    bUp->setImage(CrystalDefs::WIZ_CNT_UP_IMG);
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
    bUp->setImage(CrystalDefs::CRYS_GLOW_UP_IMG);
    bUp->setDescription(
        {"After begin struck by {i}, {i} will absorb {i}\nWhen the effect "
         "expires, their power will be multiplied",
         {PowerWizardDefs::GetIcon(), CrystalDefs::GetIcon(),
          WizardFireball::GetIcon()}});
    bUp->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                 params[CrystalParams::GlowUpCost]);
    bUp->setEffect(params[CrystalParams::GlowEffect],
                   Upgrade::Defaults::MultiplicativeEffect);
    mGlowUp = mUpgrades->subscribe(bUp);

    // Buy power wizard
    bUp = std::make_shared<Buyable>(states[State::BoughtPowerWizard]);
    bUp->setImage(WIZ_IMGS.at(POWER_WIZARD));
    bUp->setDescription(
        {"Power Wizard empowers {i} and overloads {i} for increased {i} "
         "power",
         {WizardDefs::GetIcon(), CrystalDefs::GetIcon(),
          WizardFireball::GetIcon()}});
    bUp->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                 params[CrystalParams::T1WizardCost]);
    mPowWizBuy = mUpgrades->subscribe(bUp);

    // Buy time wizard
    bUp = std::make_shared<Buyable>(states[State::BoughtTimeWizard]);
    bUp->setImage(WIZ_IMGS.at(TIME_WIZARD));
    bUp->setDescription(
        {"Time Wizard boosts {i} fire rate and freezes time for a "
         "massive power boost",
         {WizardDefs::GetIcon()}});
    bUp->setCost(Upgrade::Defaults::CRYSTAL_MAGIC,
                 params[CrystalParams::T1WizardCost]);
    mTimeWizBuy = mUpgrades->subscribe(bUp);

    // Buy catalyst
    bUp = std::make_shared<Buyable>(states[State::BoughtCatalyst]);
    bUp->setImage(WIZ_IMGS.at(CATALYST));
    bUp->setDescription(
        {"Catalyst stores magic and boosts {i} that pass nearby",
         {WizardFireball::GetIcon()}});
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
        {params[CrystalParams::NumWizards]},
        {states[State::BoughtCrysWizCntUp]},
        [this]() { return calcWizCntEffect(); }));

    mParamSubs.push_back(params[CrystalParams::GlowEffect].subscribeTo(
        {params[CrystalParams::WizardCntEffect]},
        {states[State::BoughtCrysGlowUp]},
        [this]() { return calcGlowEffect(); }));

    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[CrystalParams::Magic], params[CrystalParams::Shards]}, {},
        [this]() { drawMagic(); }));

    mParamSubs.push_back(params[CrystalParams::T1WizardCost].subscribeTo(
        states[State::BoughtFirstT1], [this](bool val) {
            return val ? CrystalDefs::T1_COST2 : CrystalDefs::T1_COST1;
        }));

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
    TextureBuilder tex;

    if (ParameterSystem::Param(State::CrysGlowActive).get()) {
        Rect glowRect = mGlowBkgrnd.getRect();
        glowRect.setPos(mPos->rect.cX(), mPos->rect.cY(), Rect::Align::CENTER);
        mGlowBkgrnd.setDest(glowRect);
        tex.draw(mGlowBkgrnd);
    }

    WizardBase::onRender(r);

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
    if (ParameterSystem::Param(State::CrysGlowActive).get()) {
        mGlowMagic += fireball.getPower();
        mGlowAnimTimerSub->get<TimerObservableBase::DATA>().timer = 0;
        mGlowAnimTimerSub->setActive(true);
    } else {
        auto magic = ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic);
        magic.set(magic.get() + fireball.getPower());
        addMessage("+" + fireball.getPower().toString(),
                   CrystalDefs::MSG_COLOR);
    }
}

void Crystal::onPowFireballHit(const PowerWizFireball& fireball) {
    createFireRing(fireball.getPower());
    ParameterSystem::States states;
    if (states[State::BoughtCrysGlowUp].get()) {
        mGlowTimerSub = TimeSystem::GetTimerObservable()->subscribe(
            [this](Timer& t) { return onGlowTimer(t); },
            Timer(fireball.getDuration().toFloat()));
        states[State::CrysGlowActive].set(true);
    }
}

bool Crystal::onGlowTimer(Timer& t) {
    ParameterSystem::Params<CRYSTAL> params;
    ParameterSystem::States states;
    Number magic = mGlowMagic * params[CrystalParams::GlowEffect].get();
    params[CrystalParams::Magic].set(params[CrystalParams::Magic].get() +
                                     magic);
    addMessage("+" + magic.toString(), CrystalDefs::GLOW_MSG_COLOR);
    mGlowMagic = 0;
    states[State::CrysGlowActive].set(false);
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
    mMagicText->setText(ss.str(), mImg.getRect().W());
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

void Crystal::addMessage(const std::string& msg, SDL_Color color) {
    mMsgTData.setText(msg).setColor(color);

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
