#include "Crystal.h"

namespace Crystal {
// Crystal
Crystal::Crystal() : WizardBase(CRYSTAL) {}

void Crystal::init() {
    mMessages = ComponentFactory<MessageHandler>::New(FONT);

    Params params;

    mImg.set(Constants::IMG());
    mImg.setDest(IMG_RECT);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);
    mGlowBkgrnd.set(Constants::GLOW_EFFECT_IMG());
    SDL_Point imgDim = mImg.get()->getTextureDim(),
              glowDim = mGlowBkgrnd.get()->getTextureDim();
    mGlowBkgrnd.setDest(Rect(0, 0, mPos->rect.w() * glowDim.x / imgDim.x,
                             mPos->rect.h() * glowDim.y / imgDim.y));
    mGlowFinishBkgrnd.set(Constants::GLOW_FINISH_IMG());
    imgDim = mImg.get()->getTextureDim();
    glowDim = mGlowFinishBkgrnd.get()->getTextureDim();
    mGlowFinishBkgrnd.setDest(Rect(0, 0, mPos->rect.w() * glowDim.x / imgDim.x,
                                   mPos->rect.h() * glowDim.y / imgDim.y));

    mMagicText->setFont(FONT).setImgs(
        {MoneyIcons::Get(params[Param::Magic]),
         MoneyIcons::Get(params[Param::Shards])});
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
    mWizFireballHitSub = Wizard::FireballList::GetHitObservable()->subscribe(
        [this](const Wizard::Fireball& fb) { onWizFireballHit(fb); },
        [this](const Wizard::Fireball& fb) {
            return Wizard::Fireball::filter(fb, mId);
        },
        mPos);
    mPowFireballHitSub =
        PowerWizard::FireballList::GetHitObservable()->subscribe(
            [this](const PowerWizard::Fireball& fb) { onPowFireballHit(fb); },
            [this](const PowerWizard::Fireball& fb) {
                return PowerWizard::Fireball::filter(fb, mId);
            },
            mPos);
    mAnimTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            mImg->nextFrame();
            WizardSystem::GetWizardImageObservable()->next(mId, mImg);
            t.length = mImg->getFrame() == 0 ? getAnimationDelay()
                                             : Constants::IMG().frame_ms;
            return true;
        },
        Constants::IMG());
    mGlowAnimTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) {
            mGlowBkgrnd->nextFrame();
            if (mGlowBkgrnd->getFrame() == 0) {
                mGlowAnimTimerSub->setActive(false);
            }
            return true;
        },
        Constants::GLOW_EFFECT_IMG());
    mPoisonTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) { return onPoisonTimer(t); }, Timer(500));
    mT1ResetSub = WizardSystem::GetWizardEventObservable()->subscribe(
        [this]() { onT1Reset(); }, WizardSystem::Event::ResetT1);
    attachSubToVisibility(mWizFireballHitSub);
    attachSubToVisibility(mPowFireballHitSub);
    attachSubToVisibility(mPoisonTimerSub);
}
void Crystal::setUpgrades() {
    Params params;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(mId);
    dUp->setDescription(
        {"Multiplier based on {i}",
         {MoneyIcons::Get(UpgradeDefaults::CRYSTAL_MAGIC)}});
    dUp->setEffects(params[Param::MagicEffect],
                    UpgradeDefaults::MultiplicativeEffect);
    mMagicEffectDisplay = mUpgrades->subscribe(dUp);

    // Wizard count upgrade
    UnlockablePtr uUp =
        std::make_shared<Unlockable>(params[Param::BoughtWizCntUp]);
    uUp->setImage(Constants::WIZ_CNT_UP_IMG);
    uUp->setDescription(
        {"Wizards synergy provides a multiplier based on the number of active "
         "wizards"});
    uUp->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                 params[Param::WizardCntUpCost]);
    uUp->setEffects(params[Param::WizardCntEffect],
                    UpgradeDefaults::MultiplicativeEffect);
    mWizCntUp = mUpgrades->subscribe(uUp);

    // Glow upgrade
    uUp = std::make_shared<Unlockable>(params[Param::BoughtGlowUp]);
    uUp->setImage(Constants::CRYS_GLOW_UP_IMG);
    uUp->setDescription(
        {"After begin struck by {i}, {i} will absorb {i}\nWhen the effect "
         "expires, their power will be multiplied",
         {IconSystem::Get(PowerWizard::Constants::IMG()),
          IconSystem::Get(Constants::IMG()),
          IconSystem::Get(Wizard::Constants::FB_IMG())}});
    uUp->setCost(UpgradeDefaults::CRYSTAL_MAGIC, params[Param::GlowUpCost]);
    uUp->setEffects(params[Param::GlowEffect],
                    UpgradeDefaults::MultiplicativeEffect);
    mGlowUp = mUpgrades->subscribe(uUp);

    // Buy power wizard
    uUp = std::make_shared<Unlockable>(params[Param::BoughtPowerWizard]);
    uUp->setImage(WIZ_IMGS.at(POWER_WIZARD));
    uUp->setDescription(
        {"Power Wizard empowers {i} and overloads {i} for increased {i} "
         "power",
         {IconSystem::Get(Wizard::Constants::IMG()),
          IconSystem::Get(Constants::IMG()),
          IconSystem::Get(Wizard::Constants::FB_IMG())}});
    uUp->setCost(UpgradeDefaults::CRYSTAL_MAGIC, params[Param::T1WizardCost]);
    mPowWizBuy = mUpgrades->subscribe(uUp);

    // Buy time wizard
    uUp = std::make_shared<Unlockable>(params[Param::BoughtTimeWizard]);
    uUp->setImage(WIZ_IMGS.at(TIME_WIZARD));
    uUp->setDescription(
        {"Time Wizard boosts {i} fire rate and freezes time for a "
         "massive power boost",
         {IconSystem::Get(Wizard::Constants::IMG())}});
    uUp->setCost(UpgradeDefaults::CRYSTAL_MAGIC, params[Param::T1WizardCost]);
    mTimeWizBuy = mUpgrades->subscribe(uUp);

    // Buy catalyst
    uUp = std::make_shared<Unlockable>(params[Param::BoughtCatalyst]);
    uUp->setImage(WIZ_IMGS.at(CATALYST));
    uUp->setDescription(
        {"Catalyst stores magic and boosts {i} that pass nearby",
         {IconSystem::Get(Wizard::Constants::FB_IMG())}});
    uUp->setCost(UpgradeDefaults::CRYSTAL_SHARDS, params[Param::CatalystCost]);
    mCatalystBuy = mUpgrades->subscribe(uUp);

    // Buy poison wizard
    uUp = std::make_shared<Unlockable>(params[Param::BoughtPoisonWizard]);
    uUp->setImage(WIZ_IMGS.at(POISON_WIZARD));
    uUp->setDescription(
        {"Poison wizard increases {i} effects and enables magic-over-time "
         "gains",
         {IconSystem::Get(Constants::IMG())}});
    uUp->setCost(UpgradeDefaults::CRYSTAL_SHARDS, params[Param::PoisonWizCost]);
    mPoisWizBuy = mUpgrades->subscribe(uUp);

    // Buy robot
    uUp = std::make_shared<Unlockable>(params[Param::BoughtRobotWizard]);
    uUp->setImage(WIZ_IMGS.at(ROBOT_WIZARD));
    uUp->setDescription({"Robot automates upgrade purchases and {i} synergies",
                         {IconSystem::Get(PowerWizard::Constants::FB_IMG())}});
    uUp->setCost(UpgradeDefaults::CRYSTAL_SHARDS, params[Param::RobotCost]);
    mRobotBuy = mUpgrades->subscribe(uUp);
}
void Crystal::setParamTriggers() {
    Params params;
    Catalyst::Params catParams;
    PoisonWizard::Params poiParams;

    mParamSubs.push_back(
        params[Param::Magic].subscribe([params](const Number& magic) {
            auto bestMagic = params[Param::BestMagic];
            if (magic > bestMagic.get()) {
                bestMagic.set(magic);
            }
        }));

    mParamSubs.push_back(params[Param::MagicEffect].subscribeTo(
        {params[Param::Magic]}, {}, [this]() { return calcMagicEffect(); }));

    mParamSubs.push_back(params[Param::ShardGain].subscribeTo(
        {params[Param::Magic], catParams[Catalyst::Param::ShardGainUp],
         poiParams[PoisonWizard::Param::ShardMultUp]},
        {}, [this]() { return calcShardGain(); }));

    mParamSubs.push_back(params[Param::NumWizards].subscribeTo(
        {},
        {params[Param::BoughtPowerWizard], params[Param::BoughtTimeWizard],
         params[Param::BoughtCatalyst]},
        [this]() { return calcNumWizards(); }));

    mParamSubs.push_back(params[Param::WizardCntEffect].subscribeTo(
        {params[Param::NumWizards]}, {params[Param::BoughtWizCntUp]},
        [this]() { return calcWizCntEffect(); }));

    mParamSubs.push_back(params[Param::GlowEffect].subscribeTo(
        {params[Param::WizardCntEffect]}, {params[Param::BoughtGlowUp]},
        [this]() { return calcGlowEffect(); }));

    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[Param::Magic], params[Param::Shards]}, {},
        [this]() { drawMagic(); }));

    mParamSubs.push_back(params[Param::T1WizardCost].subscribeTo(
        params[Param::BoughtFirstT1], [this](bool val) {
            return val ? Constants::T1_COST2 : Constants::T1_COST1;
        }));

    mParamSubs.push_back(params[Param::BoughtFirstT1].subscribeTo(
        {}, {params[Param::BoughtPowerWizard], params[Param::BoughtTimeWizard]},
        [params]() {
            return params[Param::BoughtPowerWizard].get() ||
                   params[Param::BoughtTimeWizard].get();
        }));

    mParamSubs.push_back(params[Param::BoughtSecondT1].subscribeTo(
        {}, {params[Param::BoughtPowerWizard], params[Param::BoughtTimeWizard]},
        [params]() {
            return params[Param::BoughtPowerWizard].get() &&
                   params[Param::BoughtTimeWizard].get();
        }));

    mParamSubs.push_back(params[Param::T1CostMult].subscribeTo(
        params[Param::BoughtSecondT1],
        [](bool val) { return val ? Number(1, 3) : 1; }));

    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[Param::Magic], params[Param::T1ResetCost]}, {},
        [this, params]() {
            mFractureBtn->setHidden(params[Param::Magic].get() <
                                    params[Param::T1ResetCost].get());
        }));

    // Upgrade unlock constraints
    mParamSubs.push_back(ParameterSystem::subscribe(
        {}, {params[Param::BoughtSecondT1], params[Param::BoughtWizCntUp]},
        [this, params]() {
            mGlowUp->setActive(params[Param::BoughtSecondT1].get() &&
                               params[Param::BoughtWizCntUp].get());
        }));
    mParamSubs.push_back(params[Param::ResetT1].subscribe([this](bool val) {
        mCatalystBuy->setActive(val);
        mPoisWizBuy->setActive(val);
    }));
    mParamSubs.push_back(params[Param::BoughtFirstT1].subscribe(
        [this](bool val) { mWizCntUp->setActive(val); }));
    mParamSubs.push_back(params[Param::BoughtCatalyst].subscribe(
        [this](bool bought) { mRobotBuy->setActive(bought); }));
    mParamSubs.push_back(params[Param::BoughtPoisonWizard].subscribe(
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
    if (Params::get(Param::GlowActive).get()) {
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
        auto param = Params::get(Param::Magic);
        param.set(param.get() * 3);
        param = Params::get(Param::Shards);
        param.set(param.get() * 3);

        auto p = RobotWizard::Params::get(RobotWizard::Param::Shards);
        if (p.get() == 0) {
            p.set(0.1);
        } else {
            p.set(p.get() * 3);
        }
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

void Crystal::onWizFireballHit(const Wizard::Fireball& fireball) {
    if (Params::get(Param::GlowActive).get()) {
        mGlowMagic += fireball.getPower();
        mGlowAnimTimerSub->get<TimerObservableBase::DATA>().timer = 0;
        mGlowAnimTimerSub->setActive(true);
    } else {
        addMagic(MagicSource::Fireball, fireball.getPower(),
                 Constants::MSG_COLOR);
    }

    if (fireball.isPoisoned() && Params::get(Param::BoughtPoisonWizard).get()) {
        auto poisonRate = Params::get(Param::PoisonRate);
        poisonRate.set(
            poisonRate.get() +
            PoisonWizard::Params::get(PoisonWizard::Param::PoisonFbUp).get());
    }
}

void Crystal::onPowFireballHit(const PowerWizard::Fireball& fireball) {
    createFireRing(fireball.getPower());
    if (Params::get(Param::BoughtGlowUp).get()) {
        mGlowTimerSub = TimeSystem::GetTimerObservable()->subscribe(
            [this](Timer& t) { return onGlowTimer(t); },
            Timer(fireball.getDuration().toFloat()));
        Params::get(Param::GlowActive).set(true);
    }
}

bool Crystal::onGlowTimer(Timer& t) {
    Params params;
    Number magic = mGlowMagic * params[Param::GlowEffect].get();
    mGlowMagic = 0;
    params[Param::GlowActive].set(false);

    if (mGlowFinishing) {
        mGlowFinishTimerSub->get<TimerObservable::ON_TRIGGER>()(
            mGlowFinishTimerSub->get<TimerObservable::DATA>());
    }

    mGlowFinishing = true;
    mGlowFinishTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this, magic](Timer& t) { return onGlowFinishTimer(t, magic); },
        Timer(Constants::GLOW_FINISH_IMG().frame_ms));
    return false;
}

bool Crystal::onGlowFinishTimer(Timer& t, const Number& magic) {
    mGlowFinishBkgrnd->nextFrame();
    if (mGlowFinishBkgrnd->getFrame() == 0) {
        addMagic(MagicSource::Glow, magic, Constants::GLOW_MSG_COLOR);
        mGlowFinishing = false;
        return false;
    }
    return true;
}

bool Crystal::onPoisonTimer(Timer& t) {
    Params params;
    PoisonWizard::Params poiParams;

    auto poisonRateParam = params[Param::PoisonRate];
    Number poisonRate = poisonRateParam.get();
    Number poisonMagic = params[Param::PoisonMagic].get();
    Number basePoisonRate =
        poiParams[PoisonWizard::Param::BasePoisonRate].get();
    Number poisonDecay = poiParams[PoisonWizard::Param::PoisonDecay].get();

    float s = (float)t.length / 1000;
    addMagic(MagicSource::Poison, poisonMagic * poisonRate * s,
             Constants::POISON_MSG_COLOR);
    if (poisonRate > basePoisonRate) {
        poisonRateParam.set(
            max(basePoisonRate, poisonRate * (poisonDecay ^ s)));
    } else if (poisonRate < basePoisonRate) {
        poisonRateParam.set(basePoisonRate);
    }

    return true;
}

Number Crystal::calcMagicEffect() {
    Params params;
    return (params[Param::Magic].get() + 1).logTen() + 1;
}

Number Crystal::calcShardGain() {
    Params params;
    Catalyst::Params catParams;
    PoisonWizard::Params poiParams;

    Number shards = ((params[Param::Magic].get() + 1).logTen() - 14) *
                    catParams[Catalyst::Param::ShardGainUp].get() *
                    poiParams[PoisonWizard::Param::ShardMultUp].get();
    if (shards < 1) {
        return 0;
    }
    return shards;
}

Number Crystal::calcNumWizards() {
    // Start with wizard
    int cnt = 1;
    for (auto state : {Param::BoughtPowerWizard, Param::BoughtTimeWizard,
                       Param::BoughtCatalyst, Param::BoughtPoisonWizard,
                       Param::BoughtRobotWizard}) {
        cnt += Params::get(state).get();
    }

    return cnt;
}

Number Crystal::calcWizCntEffect() {
    if (!Params::get(Param::BoughtWizCntUp).get()) {
        return 1;
    }

    return Params::get(Param::NumWizards).get() ^ 2;
}

Number Crystal::calcGlowEffect() {
    Params params;

    if (!params[Param::BoughtGlowUp].get()) {
        return 1;
    }

    return params[Param::WizardCntEffect].get() ^ 1.5;
}

void Crystal::drawMagic() {
    Params params;
    std::stringstream ss;
    ss << "{i}" << params[Param::Magic].get();
    if (Params::get(Param::Shards).get() > 0) {
        ss << "\n{i}" << params[Param::Shards].get();
    }
    mMagicText->setText(ss.str(), mImg.getRect().W());
}

void Crystal::addMagic(MagicSource source, const Number& amnt,
                       SDL_Color msgColor) {
    auto magic = Params::get(Param::Magic);
    magic.set(magic.get() + amnt);
    mMessages->addMessage(mPos->rect, "+" + amnt.toString(), msgColor);

    auto poisonMagic = Params::get(Param::PoisonMagic);
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
    auto magic = Params::get(Param::Magic);
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
}  // namespace Crystal
