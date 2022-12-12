#include "Catalyst.h"

// Catalyst
Catalyst::Catalyst() : WizardBase(CATALYST) {
    mPos->elevation = Elevation::CATALYST;
}

void Catalyst::init() {
    mMessages = ComponentFactory<MessageHandler>::New(FONT);

    ParameterSystem::Params<CATALYST> params;
    mImg.set(CatalystDefs::IMG);
    mImg.setDest(IMG_RECT);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);

    mMagicText->setFont(FONT);
    mMagicText->setImgs({Money::GetMoneyIcon(params[CatalystParams::Magic])});
    mMagicRender.set(mMagicText);
    mMagicRender.setFit(RenderData::FitMode::Texture);
    mMagicRender.setFitAlign(Rect::Align::CENTER, Rect::Align::TOP_LEFT);

    mRange = CircleShape(PURPLE).setDashed(50);

    setPos(mPos->rect.cX(), mPos->rect.cY());

    WizardBase::init();
}
void Catalyst::setSubscriptions() {
    mWizFireballSub = WizardFireball::GetHitObservable()->subscribe(
        [this](const WizardFireball& f) { onWizFireballHit(f); }, mId);
    mPoisFireballSub = PoisonFireball::GetHitObservable()->subscribe(
        [this](const PoisonFireball& f) { onPoisFireballHit(f); }, mId);
    mMagicSub = WizardSystem::GetCatalystMagicObservable()->subscribe(
        [this](const Number& amnt) { onMagic(amnt); });
    attachSubToVisibility(mWizFireballSub);
}
void Catalyst::setUpgrades() {
    ParameterSystem::Params<CATALYST> params;
    ParameterSystem::States states;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(WIZ_IMGS.at(mId));
    dUp->setEffects(
        {params[CatalystParams::MagicEffect],
         params[CatalystParams::FBCntEffect], params[CatalystParams::Range]},
        {}, []() -> TextUpdateData {
            ParameterSystem::Params<CATALYST> params;

            std::stringstream ss;
            ss << "{i} Multiplier: "
               << UpgradeDefaults::MultiplicativeEffectText(
                      params[CatalystParams::MagicEffect].get())
               << "\n{i} Multiplier: "
               << UpgradeDefaults::MultiplicativeEffectText(
                      params[CatalystParams::FBCntEffect].get())
               << "\nRange: " << params[CatalystParams::Range].get();
            return {ss.str(),
                    {Money::GetMoneyIcon(UpgradeDefaults::CATALYST_MAGIC),
                     IconSystem::Get(WizardDefs::FB_IMG)}};
        });
    mMagicEffectDisplay = mUpgrades->subscribe(dUp);

    // Zap count
    UpgradePtr up =
        std::make_shared<Upgrade>(params[CatalystParams::ZapCntUpLvl], 3);
    up->setImage("");
    up->setDescription({"Increases the number of times each {i} can be zapped",
                        {IconSystem::Get(WizardDefs::FB_IMG)}});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                params[CatalystParams::ZapCntUpCost]);
    up->setEffects(params[CatalystParams::ZapCntUp], UpgradeDefaults::NoEffect);
    mParamSubs.push_back(params[CatalystParams::ZapCntUpCost].subscribeTo(
        up->level(), [](const Number& lvl) { return 10 ^ (10 * (lvl + 1)); }));
    mParamSubs.push_back(params[CatalystParams::ZapCntUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1 + lvl; }));
    mZapCntUp = mUpgrades->subscribe(up);

    // Num zappers
    up = std::make_shared<Upgrade>(params[CatalystParams::ZapperUpLvl], 4);
    up->setImage("");
    up->setDescription({"Increases number and charge rate of zaps"});
    up->setCost(UpgradeDefaults::CRYSTAL_SHARDS,
                params[CatalystParams::ZapperUpCost]);
    up->setEffects(params[CatalystParams::ZapperUp], UpgradeDefaults::NoEffect);
    mParamSubs.push_back(params[CatalystParams::ZapperUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1 + lvl; }));
    mParamSubs.push_back(params[CatalystParams::ZapperUpCost].subscribeTo(
        up->level(), [](const Number& lvl) { return lvl * (lvl + 1); }));
    mZapperCntUp = mUpgrades->subscribe(up);

    // Zap range
    up = std::make_shared<Upgrade>(params[CatalystParams::RangeUpLvl], 5);
    up->setImage("");
    up->setDescription({"Increase range of fireball boost"});
    up->setCost(UpgradeDefaults::CRYSTAL_SHARDS,
                params[CatalystParams::RangeUpCost]);
    up->setEffects(params[CatalystParams::RangeUp],
                   UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(params[CatalystParams::RangeUpCost].subscribeTo(
        up->level(), [](const Number& lvl) { return 5 * (2 ^ lvl); }));
    mParamSubs.push_back(params[CatalystParams::RangeUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1.1 ^ lvl; }));
    mRangeUp = mUpgrades->subscribe(up);

    // Boost shard gain
    UnlockablePtr uUp =
        std::make_shared<Unlockable>(states[State::BoughtCatShardMult]);
    uUp->setImage("");
    uUp->setDescription(
        {"{i} effect boosts {i} gain",
         {IconSystem::Get(CatalystDefs::IMG),
          Money::GetMoneyIcon(UpgradeDefaults::CRYSTAL_SHARDS)}});
    uUp->setCost(UpgradeDefaults::CRYSTAL_SHARDS,
                 params[CatalystParams::ShardGainUpCost]);
    uUp->setEffects(params[CatalystParams::ShardGainUp],
                    UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(params[CatalystParams::ShardGainUp].subscribeTo(
        {params[CatalystParams::MagicEffect]}, {uUp->level()}, []() -> Number {
            if (!ParameterSystem::Param(State::BoughtCatShardMult).get()) {
                return 1;
            }
            return ParameterSystem::Param<CATALYST>(CatalystParams::MagicEffect)
                .get();
        }));
    mShardUp = mUpgrades->subscribe(uUp);

    // Boost catalyst gain
    up = std::make_shared<Upgrade>(params[CatalystParams::GainUp1Lvl], 10);
    up->setImage("");
    up->setDescription(
        {"Multiplies {i} gain *2", {IconSystem::Get(CatalystDefs::IMG)}});
    up->setCost(UpgradeDefaults::CRYSTAL_SHARDS,
                params[CatalystParams::GainUp1Cost]);
    up->setEffects(params[CatalystParams::GainUp1],
                   UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(params[CatalystParams::GainUp1Cost].subscribeTo(
        up->level(), [](const Number& lvl) { return 10 * (2 ^ lvl); }));
    mParamSubs.push_back(params[CatalystParams::GainUp1].subscribeTo(
        up->level(), [](const Number& lvl) { return 2 ^ lvl; }));
    mGainUp1 = mUpgrades->subscribe(up);

    // Boost catalyst gain 2
    up = std::make_shared<Upgrade>(params[CatalystParams::GainUp2Lvl], 5);
    up->setImage("");
    up->setDescription(
        {"Improves {i} gain formula", {IconSystem::Get(CatalystDefs::IMG)}});
    up->setCost(UpgradeDefaults::CRYSTAL_SHARDS,
                params[CatalystParams::GainUp2Cost]);
    up->setEffects(
        params[CatalystParams::GainUp2],
        [](const Number& effect) -> TextUpdateData {
            return {"log(x) -> x" + UpgradeDefaults::PowerEffectText(effect)};
        });
    mParamSubs.push_back(params[CatalystParams::GainUp2Cost].subscribeTo(
        up->level(),
        [](const Number& lvl) { return Number(1, 4) * (5 ^ lvl); }));
    mParamSubs.push_back(params[CatalystParams::GainUp2].subscribeTo(
        up->level(), [](const Number& lvl) { return lvl / 10; }));
    mGainUp2 = mUpgrades->subscribe(up);

    // Fireball count upgrade
    up = std::make_shared<Upgrade>(params[CatalystParams::FBCntLvl],
                                   params[CatalystParams::FBCntMaxLvl]);
    up->setImage("");
    mParamSubs.push_back(params[CatalystParams::FBCntMaxLvl].subscribeTo(
        {},
        {states[State::BoughtPoisonWizard], states[State::BoughtRobotWizard]},
        [states]() {
            return states[State::BoughtRobotWizard].get()    ? 3
                   : states[State::BoughtPoisonWizard].get() ? 2
                                                             : 1;
        }));
    mParamSubs.push_back(params[CatalystParams::FBCntUpCost].subscribeTo(
        up->level(), [up, params](const Number& lvl) {
            up->setCost(CatalystDefs::FB_CNT_TYPES.at(lvl.toInt()),
                        params[CatalystParams::FBCntUpCost]);
            switch (lvl.toInt()) {
                case CatalystDefs::FBCntUpBuyType::Reg:
                    return 1000;
                case CatalystDefs::FBCntUpBuyType::Pow:
                    return 100;
                case CatalystDefs::FBCntUpBuyType::Poi:
                    return 50;
            };
            return 0;
        }));
    mParamSubs.push_back(ParameterSystem::subscribe(
        {CatalystDefs::REG_FB_CNT, CatalystDefs::POW_FB_CNT,
         CatalystDefs::POI_FB_CNT, up->level()},
        {}, [up]() {
            int lvl = up->level().get().toInt();
            std::stringstream ss;
            for (int i = 0; i < lvl; i++) {
                if (i > 0) {
                    ss << " | ";
                }
                ss << CatalystDefs::FB_CNT_TYPES.at(i + 1).get() << "{i}";
            }
            up->setEffectText(ss.str());
        }));
    mParamSubs.push_back(ParameterSystem::subscribe(
        {up->level(), params[CatalystParams::FBCntMaxLvl]}, {}, [up, params]() {
            int lvl = up->level().get().toInt();
            int maxLvl = params[CatalystParams::FBCntMaxLvl].get().toInt();

            std::vector<RenderTextureCPtr> imgs;
            std::stringstream desc_str;
            desc_str << "Hitting {i} with {i}";
            for (int i = 0; i < lvl; i++) {
                if (i < maxLvl - 1) {
                    desc_str << ",{i}";
                }
                imgs.push_back(
                    Money::GetMoneyIcon(CatalystDefs::FB_CNT_TYPES.at(i + 1)));
            }
            desc_str << " boosts {i} power";
            up->setEffectImgs(imgs);

            imgs.insert(imgs.begin(), IconSystem::Get(CatalystDefs::IMG));
            if (lvl < maxLvl) {
                imgs.push_back(Money::GetMoneyIcon(
                    CatalystDefs::FB_CNT_TYPES.at(lvl + 1)));
            }
            imgs.push_back(IconSystem::Get(WizardDefs::IMG));

            up->setDescription({desc_str.str(), imgs});
        }));
    mFbCountUp = mUpgrades->subscribe(up);
}
void Catalyst::setParamTriggers() {
    ParameterSystem::Params<CATALYST> params;
    ParameterSystem::States states;

    mParamSubs.push_back(params[CatalystParams::MagicEffect].subscribeTo(
        {params[CatalystParams::Magic]}, {},
        [this]() { return calcMagicEffect(); }));

    mParamSubs.push_back(params[CatalystParams::Range].subscribeTo(
        {params[CatalystParams::BaseRange], params[CatalystParams::RangeUp]},
        {}, [this]() { return calcRange(); }));
    mParamSubs.push_back(params[CatalystParams::FBCntEffect].subscribeTo(
        {CatalystDefs::REG_FB_CNT, CatalystDefs::POW_FB_CNT,
         CatalystDefs::POI_FB_CNT, params[CatalystParams::FBCntLvl]},
        {}, [this]() { return calcFbCntEffect(); }));

    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[CatalystParams::Magic], params[CatalystParams::Capacity]}, {},
        [this]() { drawMagic(); }));

    mParamSubs.push_back(ParameterSystem::Param(State::BoughtCatalyst)
                             .subscribe([this](bool bought) {
                                 WizardSystem::GetHideObservable()->next(
                                     mId, !bought);
                             }));

    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[CatalystParams::Range]}, {}, [this]() { updateRange(); }));

    mParamSubs.push_back(
        params[CatalystParams::GainUp1Lvl].subscribe([this](const Number& lvl) {
            mGainUp2->setActive(mGainUp1->get<UpgradeList::DATA>()->status() ==
                                Upgrade::BOUGHT);
        }));
}

void Catalyst::onWizFireballHit(const WizardFireball& fireball) {
    WizardSystem::GetCatalystMagicObservable()->next(fireball);

    ParameterSystem::Params<CATALYST> params;
    bool buffed = fireball.isBoosted(), poisoned = fireball.isPoisoned();
    if (buffed || poisoned) {
        if (buffed) {
            auto cnt = CatalystDefs::POW_FB_CNT;
            cnt.set(cnt.get() + 1);
        }
        if (poisoned) {
            auto cnt = CatalystDefs::POI_FB_CNT;
            cnt.set(cnt.get() + 1);
        }
    } else {
        auto cnt = CatalystDefs::REG_FB_CNT;
        cnt.set(cnt.get() + 1);
    }
}
void Catalyst::onPoisFireballHit(const PoisonFireball& fireball) {
    auto poisCnt =
        ParameterSystem::Param<CATALYST>(CatalystParams::CatRingPoisCnt);
    Number poisCntUp =
        ParameterSystem::Param<POISON_WIZARD>(PoisonWizardParams::CatPoisCntUp)
            .get();

    poisCnt.set(poisCnt.get() + poisCntUp);
}

void Catalyst::onMagic(const Number& amnt) {
    ParameterSystem::Params<CATALYST> params;
    auto magic = params[CatalystParams::Magic];
    magic.set(max(
        0, min(magic.get() + amnt, params[CatalystParams::Capacity].get())));
    mMessages->addMessage(mPos->rect, "+" + amnt.toString(), RED);
}

void Catalyst::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    TextureBuilder tex;

    tex.draw(mRange);

    mMessages->draw(tex);

    mMagicRender.setDest(
        Rect(mPos->rect.x(), mPos->rect.y2(), mPos->rect.w(), FONT.h));
    tex.draw(mMagicRender);
}

Number Catalyst::calcMagicEffect() {
    ParameterSystem::Params<CATALYST> params;
    return (params[CatalystParams::Magic].get() ^ .5) + 1;
}

Number Catalyst::calcRange() {
    ParameterSystem::Params<CATALYST> params;
    return params[CatalystParams::BaseRange].get() *
           params[CatalystParams::RangeUp].get();
}

Number Catalyst::calcFbCntEffect() {
    ParameterSystem::Params<CATALYST> params;
    Number reg_effect = 1, pow_effect = 1, poi_effect = 1;
    int lvl = params[CatalystParams::FBCntLvl].get().toInt();
    if (lvl > CatalystDefs::FBCntUpBuyType::Reg) {
        reg_effect = CatalystDefs::REG_FB_CNT.get() + 1;
        if (lvl > CatalystDefs::FBCntUpBuyType::Poi) {
            poi_effect = (CatalystDefs::POI_FB_CNT.get() + 1).sqrt();
            if (lvl > CatalystDefs::FBCntUpBuyType::Pow) {
                pow_effect = (CatalystDefs::POW_FB_CNT.get() + 10).logTen();
            }
        }
    }
    return reg_effect * ((poi_effect * pow_effect) ^ pow_effect);
}

void Catalyst::drawMagic() {
    ParameterSystem::Params<CATALYST> params;
    std::stringstream ss;
    ss << "{i} " << params[CatalystParams::Magic].get() << "/{b}"
       << params[CatalystParams::Capacity].get();
    mMagicText->setText(ss.str(), mPos->rect.W());
}

void Catalyst::updateRange() {
    float half = fmaxf(mPos->rect.halfH(), mPos->rect.halfW());
    auto range = ParameterSystem::Param<CATALYST>(CatalystParams::Range);
    mRange.setRadius((int)(half * range.get().toFloat()), ceilf(half / 100),
                     true);
    CatalystRing::GetHitObservable()->setPos(mRange.get());
}

void Catalyst::setPos(float x, float y) {
    WizardBase::setPos(x, y);

    mRange.setCenter({mPos->rect.CX(), mPos->rect.CY()});
    CatalystRing::GetHitObservable()->setPos(mRange.get());
}
