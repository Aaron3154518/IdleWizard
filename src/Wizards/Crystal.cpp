#include "Crystal.h"

// Crystal
Crystal::Crystal() : WizardBase(CRYSTAL) {}

void Crystal::init() {
    mMessages = ComponentFactory<MessageHandler>::New(FONT);

    ParameterSystem::Params<CRYSTAL> params;

    mImg.set(CrystalDefs::IMG());
    mImg.setDest(IMG_RECT);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);
    mGlowBkgrnd.set(CrystalDefs::GLOW_EFFECT_IMG());
    SDL_Point imgDim = mImg.get()->getTextureDim(),
              glowDim = mGlowBkgrnd.get()->getTextureDim();
    mGlowBkgrnd.setDest(Rect(0, 0, mPos->rect.w() * glowDim.x / imgDim.x,
                             mPos->rect.h() * glowDim.y / imgDim.y));
    mGlowFinishBkgrnd.set(CrystalDefs::GLOW_FINISH_IMG());
    imgDim = mImg.get()->getTextureDim();
    glowDim = mGlowFinishBkgrnd.get()->getTextureDim();
    mGlowFinishBkgrnd.setDest(Rect(0, 0, mPos->rect.w() * glowDim.x / imgDim.x,
                                   mPos->rect.h() * glowDim.y / imgDim.y));

    mMagicText->setFont(FONT).setImgs(
        {Money::GetMoneyIcon(params[CrystalParams::Magic]),
         Money::GetMoneyIcon(params[CrystalParams::Shards])});
    mMagicRender.set(mMagicText);
    mMagicRender.setFit(RenderData::FitMode::Texture);
    mMagicRender.setFitAlign(Rect::Align::CENTER, Rect::Align::TOP_LEFT);

    mFractureBtn = ComponentFactory<FractureButton>::New();
    mFractureBtn->setHidden(true);

    WizardBase::init();

    SDL_Point screenDim = RenderSystem::getWindowSize();
    setPos(screenDim.x / 2, screenDim.y / 2);
}
void Crystal::setSubscriptions() {
    mWizFireballHitSub = WizardFireball::GetHitObservable()->subscribe(
        [this](const WizardFireball& f) { onWizFireballHit(f); }, mId);
    mPowFireballHitSub = PowerFireball::GetHitObservable()->subscribe(
        [this](const PowerFireball& f) { onPowFireballHit(f); }, mId);
    mAnimTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            mImg->nextFrame();
            WizardSystem::GetWizardImageObservable()->next(mId, mImg);
            t.length = mImg->getFrame() == 0 ? getAnimationDelay()
                                             : CrystalDefs::IMG().frame_ms;
            return true;
        },
        CrystalDefs::IMG());
    mGlowAnimTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            mGlowBkgrnd->nextFrame();
            if (mGlowBkgrnd->getFrame() == 0) {
                mGlowAnimTimerSub->setActive(false);
            }
            return true;
        },
        CrystalDefs::GLOW_EFFECT_IMG());
    mPoisonTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) { return onPoisonTimer(t); }, Timer(500));
    mT1ResetSub = WizardSystem::GetWizardEventObservable()->subscribe(
        [this]() { onT1Reset(); }, WizardSystem::Event::ResetT1);
    attachSubToVisibility(mWizFireballHitSub);
    attachSubToVisibility(mPowFireballHitSub);
    attachSubToVisibility(mPoisonTimerSub);
}
void Crystal::setUpgrades() {
    ParameterSystem::Params<CRYSTAL> params;
    ParameterSystem::States states;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(mId);
    dUp->setDescription(
        {"Multiplier based on {i}",
         {Money::GetMoneyIcon(UpgradeDefaults::CRYSTAL_MAGIC)}});
    dUp->setEffects(params[CrystalParams::MagicEffect],
                    UpgradeDefaults::MultiplicativeEffect);
    mMagicEffectDisplay = mUpgrades->subscribe(dUp);

    // Wizard count upgrade
    UnlockablePtr uUp =
        std::make_shared<Unlockable>(states[State::BoughtCrysWizCntUp]);
    uUp->setImage(CrystalDefs::WIZ_CNT_UP_IMG);
    uUp->setDescription(
        {"Wizards synergy provides a multiplier based on the number of active "
         "wizards"});
    uUp->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                 params[CrystalParams::WizardCntUpCost]);
    uUp->setEffects(params[CrystalParams::WizardCntEffect],
                    UpgradeDefaults::MultiplicativeEffect);
    mWizCntUp = mUpgrades->subscribe(uUp);

    // Glow upgrade
    uUp = std::make_shared<Unlockable>(states[State::BoughtCrysGlowUp]);
    uUp->setImage(CrystalDefs::CRYS_GLOW_UP_IMG);
    uUp->setDescription(
        {"After begin struck by {i}, {i} will absorb {i}\nWhen the effect "
         "expires, their power will be multiplied",
         {IconSystem::Get(PowerWizardDefs::IMG()),
          IconSystem::Get(CrystalDefs::IMG()),
          IconSystem::Get(WizardDefs::FB_IMG())}});
    uUp->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                 params[CrystalParams::GlowUpCost]);
    uUp->setEffects(params[CrystalParams::GlowEffect],
                    UpgradeDefaults::MultiplicativeEffect);
    mGlowUp = mUpgrades->subscribe(uUp);

    // Buy power wizard
    uUp = std::make_shared<Unlockable>(states[State::BoughtPowerWizard]);
    uUp->setImage(WIZ_IMGS.at(POWER_WIZARD));
    uUp->setDescription(
        {"Power Wizard empowers {i} and overloads {i} for increased {i} "
         "power",
         {IconSystem::Get(WizardDefs::IMG()),
          IconSystem::Get(CrystalDefs::IMG()),
          IconSystem::Get(WizardDefs::FB_IMG())}});
    uUp->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                 params[CrystalParams::T1WizardCost]);
    mPowWizBuy = mUpgrades->subscribe(uUp);

    // Buy time wizard
    uUp = std::make_shared<Unlockable>(states[State::BoughtTimeWizard]);
    uUp->setImage(WIZ_IMGS.at(TIME_WIZARD));
    uUp->setDescription(
        {"Time Wizard boosts {i} fire rate and freezes time for a "
         "massive power boost",
         {IconSystem::Get(WizardDefs::IMG())}});
    uUp->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                 params[CrystalParams::T1WizardCost]);
    mTimeWizBuy = mUpgrades->subscribe(uUp);

    // Buy catalyst
    uUp = std::make_shared<Unlockable>(states[State::BoughtCatalyst]);
    uUp->setImage(WIZ_IMGS.at(CATALYST));
    uUp->setDescription(
        {"Catalyst stores magic and boosts {i} that pass nearby",
         {IconSystem::Get(WizardDefs::FB_IMG())}});
    uUp->setCost(UpgradeDefaults::CRYSTAL_SHARDS,
                 params[CrystalParams::CatalystCost]);
    mCatalystBuy = mUpgrades->subscribe(uUp);

    // Buy poison wizard
    uUp = std::make_shared<Unlockable>(states[State::BoughtPoisonWizard]);
    uUp->setImage(WIZ_IMGS.at(POISON_WIZARD));
    uUp->setDescription(
        {"Poison wizard increases {i} effects and enables magic-over-time "
         "gains",
         {IconSystem::Get(CrystalDefs::IMG())}});
    uUp->setCost(UpgradeDefaults::CRYSTAL_SHARDS,
                 params[CrystalParams::PoisonWizCost]);
    mPoisWizBuy = mUpgrades->subscribe(uUp);

    // Buy robot
    uUp = std::make_shared<Unlockable>(states[State::BoughtRobotWizard]);
    uUp->setImage(WIZ_IMGS.at(ROBOT_WIZARD));
    uUp->setDescription({"Robot automates upgrade purchases and {i} synergies",
                         {IconSystem::Get(PowerWizardDefs::FB_IMG())}});
    uUp->setCost(UpgradeDefaults::CRYSTAL_SHARDS,
                 params[CrystalParams::RobotCost]);
    mRobotBuy = mUpgrades->subscribe(uUp);
}
void Crystal::setParamTriggers() {
    ParameterSystem::Params<CRYSTAL> params;
    ParameterSystem::Params<CATALYST> catParams;
    ParameterSystem::Params<POISON_WIZARD> poiParams;
    ParameterSystem::States states;

    mParamSubs.push_back(
        params[CrystalParams::Magic].subscribe([params](const Number& magic) {
            auto bestMagic = params[CrystalParams::BestMagic];
            if (magic > bestMagic.get()) {
                bestMagic.set(magic);
            }
        }));

    mParamSubs.push_back(params[CrystalParams::MagicEffect].subscribeTo(
        {params[CrystalParams::Magic]}, {},
        [this]() { return calcMagicEffect(); }));

    mParamSubs.push_back(params[CrystalParams::ShardGain].subscribeTo(
        {params[CrystalParams::Magic], catParams[CatalystParams::ShardGainUp],
         poiParams[PoisonWizardParams::ShardMultUp]},
        {}, [this]() { return calcShardGain(); }));

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

    mParamSubs.push_back(params[CrystalParams::T1CostMult].subscribeTo(
        states[State::BoughtSecondT1],
        [](bool val) { return val ? Number(1, 3) : 1; }));

    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[CrystalParams::Magic], params[CrystalParams::T1ResetCost]}, {},
        [this, params]() {
            mFractureBtn->setHidden(params[CrystalParams::Magic].get() <
                                    params[CrystalParams::T1ResetCost].get());
        }));

    // Upgrade unlock constraints
    mParamSubs.push_back(ParameterSystem::subscribe(
        {}, {states[State::BoughtSecondT1], states[State::BoughtCrysWizCntUp]},
        [this]() {
            ParameterSystem::States states;
            mGlowUp->setActive(states[State::BoughtSecondT1].get() &&
                               states[State::BoughtCrysWizCntUp].get());
        }));
    mParamSubs.push_back(states[State::ResetT1].subscribe([this](bool val) {
        mCatalystBuy->setActive(val);
        mPoisWizBuy->setActive(val);
    }));
    mParamSubs.push_back(states[State::BoughtFirstT1].subscribe(
        [this](bool val) { mWizCntUp->setActive(val); }));
    mParamSubs.push_back(states[State::BoughtCatalyst].subscribe(
        [this](bool bought) { mRobotBuy->setActive(bought); }));
    mParamSubs.push_back(states[State::BoughtPoisonWizard].subscribe(
        [this, params](bool bought) { mPoisonTimerSub->setActive(bought); }));
}

void Crystal::onRender(SDL_Renderer* r) {
    TextureBuilder tex;

    if (mGlowFinishing) {
        Rect glowRect = mGlowFinishBkgrnd.getRect();
        glowRect.setPos(mPos->rect.cX(), mPos->rect.cY(), Rect::Align::CENTER);
        mGlowFinishBkgrnd.setDest(glowRect);
        tex.draw(mGlowFinishBkgrnd);
    }
    if (ParameterSystem::Param(State::CrysGlowActive).get()) {
        Rect glowRect = mGlowBkgrnd.getRect();
        glowRect.setPos(mPos->rect.cX(), mPos->rect.cY(), Rect::Align::CENTER);
        mGlowBkgrnd.setDest(glowRect);
        tex.draw(mGlowBkgrnd);
    }

    WizardBase::onRender(r);

    tex.draw(mMagicRender);

    mMessages->draw(tex);

    for (auto it = mFireRings.begin(); it != mFireRings.end(); ++it) {
        if ((*it)->dead()) {
            it = mFireRings.erase(it);
            if (it == mFireRings.end()) {
                break;
            }
        }
    }

    if (!mFractureBtn->isHidden()) {
        mFractureBtn->onRender(r);
    }
}

void Crystal::onClick(Event::MouseButton b, bool clicked) {
    static bool _addMagic = false;
    if (_addMagic && clicked) {
        auto param = ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic);
        param.set(param.get() * 3);
        param = ParameterSystem::Param<CRYSTAL>(CrystalParams::Shards);
        param.set(param.get() * 3);
    }
    _addMagic = clicked;
    WizardBase::onClick(b, clicked);
}

void Crystal::onHide(bool hide) {
    WizardBase::onHide(hide);
    if (hide) {
        mFireRings.clear();
    }
}

void Crystal::onT1Reset() {
    mFireRings.clear();
    mGlowTimerSub.reset();
    mGlowFinishing = false;
    mGlowFinishTimerSub.reset();
    mGlowAnimTimerSub.reset();
}

void Crystal::onWizFireballHit(const WizardFireball& fireball) {
    if (ParameterSystem::Param(State::CrysGlowActive).get()) {
        mGlowMagic += fireball.getPower();
        mGlowAnimTimerSub->get<TimerObservableBase::DATA>().timer = 0;
        mGlowAnimTimerSub->setActive(true);
    } else {
        addMagic(MagicSource::Fireball, fireball.getPower(),
                 CrystalDefs::MSG_COLOR);
    }

    if (fireball.isPoisoned() &&
        ParameterSystem::Param(State::BoughtPoisonWizard).get()) {
        auto poisonRate =
            ParameterSystem::Param<CRYSTAL>(CrystalParams::PoisonRate);
        poisonRate.set(poisonRate.get() + ParameterSystem::Param<POISON_WIZARD>(
                                              PoisonWizardParams::PoisonFbUp)
                                              .get());
    }
}

void Crystal::onPowFireballHit(const PowerFireball& fireball) {
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
    mGlowMagic = 0;
    states[State::CrysGlowActive].set(false);

    if (mGlowFinishing) {
        mGlowFinishTimerSub->get<TimerObservable::ON_TRIGGER>()(
            mGlowFinishTimerSub->get<TimerObservable::DATA>());
    }

    mGlowFinishing = true;
    mGlowFinishTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this, magic](Timer& t) { return onGlowFinishTimer(t, magic); },
        Timer(CrystalDefs::GLOW_FINISH_IMG().frame_ms));
    return false;
}

bool Crystal::onGlowFinishTimer(Timer& t, const Number& magic) {
    mGlowFinishBkgrnd->nextFrame();
    if (mGlowFinishBkgrnd->getFrame() == 0) {
        addMagic(MagicSource::Glow, magic, CrystalDefs::GLOW_MSG_COLOR);
        mGlowFinishing = false;
        return false;
    }
    return true;
}

bool Crystal::onPoisonTimer(Timer& t) {
    ParameterSystem::Params<CRYSTAL> params;
    ParameterSystem::Params<POISON_WIZARD> poiParams;

    auto poisonRateParam = params[CrystalParams::PoisonRate];
    Number poisonRate = poisonRateParam.get();
    Number poisonMagic = params[CrystalParams::PoisonMagic].get();
    Number basePoisonRate = poiParams[PoisonWizardParams::BasePoisonRate].get();
    Number poisonDecay = poiParams[PoisonWizardParams::PoisonDecay].get();

    float s = (float)t.length / 1000;
    addMagic(MagicSource::Poison, poisonMagic * poisonRate * s,
             CrystalDefs::POISON_MSG_COLOR);
    if (poisonRate > basePoisonRate) {
        poisonRateParam.set(
            max(basePoisonRate, poisonRate * (poisonDecay ^ s)));
    } else if (poisonRate < basePoisonRate) {
        poisonRateParam.set(basePoisonRate);
    }

    return true;
}

Number Crystal::calcMagicEffect() {
    ParameterSystem::Params<CRYSTAL> params;
    return (params[CrystalParams::Magic].get() + 1).logTen() + 1;
}

Number Crystal::calcShardGain() {
    ParameterSystem::Params<CRYSTAL> params;
    ParameterSystem::Params<CATALYST> catParams;
    ParameterSystem::Params<POISON_WIZARD> poiParams;

    Number shards = ((params[CrystalParams::Magic].get() + 1).logTen() - 14) *
                    catParams[CatalystParams::ShardGainUp].get() *
                    poiParams[PoisonWizardParams::ShardMultUp].get();
    if (shards < 1) {
        return 0;
    }
    return shards;
}

Number Crystal::calcNumWizards() {
    ParameterSystem::States states;

    // Start with wizard
    int cnt = 1;
    for (auto state : {State::BoughtPowerWizard, State::BoughtTimeWizard,
                       State::BoughtCatalyst, State::BoughtPoisonWizard,
                       State::BoughtRobotWizard}) {
        cnt += states[state].get();
    }

    return cnt;
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
    if (ParameterSystem::Param<CRYSTAL>(CrystalParams::Shards).get() > 0) {
        ss << "\n{i}" << params[CrystalParams::Shards].get();
    }
    mMagicText->setText(ss.str(), mImg.getRect().W());
}

void Crystal::addMagic(MagicSource source, const Number& amnt,
                       SDL_Color msgColor) {
    auto magic = ParameterSystem::Param<CRYSTAL>(CrystalParams::Magic);
    magic.set(magic.get() + amnt);
    mMessages->addMessage(mPos->rect, "+" + amnt.toString(), msgColor);

    auto poisonMagic =
        ParameterSystem::Param<CRYSTAL>(CrystalParams::PoisonMagic);
    switch (source) {
        case MagicSource::Fireball:
        case MagicSource::Glow:
            if (amnt > poisonMagic.get()) {
                poisonMagic.set(amnt);
            }
            break;
    };
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

void Crystal::setPos(float x, float y) {
    std::cerr << x << " " << y << std::endl;
    WizardBase::setPos(x, y);

    mMagicRender.setDest(
        Rect(mPos->rect.x(), mPos->rect.y2(), mPos->rect.w(), FONT.h * 2));
}
