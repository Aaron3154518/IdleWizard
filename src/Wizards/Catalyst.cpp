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

    mFbCntText->setFont(FONT);
    mFbCntRender.set(mFbCntText);
    mFbCntRender.setFit(RenderData::FitMode::Texture);
    mFbCntRender.setFitAlign(Rect::Align::CENTER, Rect::Align::TOP_LEFT);

    mRange = CircleShape(PURPLE).setDashed(50);

    setPos(mPos->rect.cX(), mPos->rect.cY());

    WizardBase::init();
}
void Catalyst::setSubscriptions() {
    mWizFireballSub = WizardFireball::GetHitObservable()->subscribe(
        [this](const WizardFireball& f) { onWizFireballHit(f); }, mId);
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
        {params[CatalystParams::MagicEffect], params[CatalystParams::Range]},
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
    up->setEffects(params[CatalystParams::ZapCnt], UpgradeDefaults::NoEffect);
    mParamSubs.push_back(params[CatalystParams::ZapCntUpCost].subscribeTo(
        up->level(), [](const Number& lvl) { return 10 ^ (10 * (lvl + 1)); }));
    mParamSubs.push_back(params[CatalystParams::ZapCnt].subscribeTo(
        up->level(), [](const Number& lvl) { return 1 + lvl; }));
    mZapCntUp = mUpgrades->subscribe(up);

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

    // Boost catalyst gain
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
        {params[CatalystParams::FBRegCnt], params[CatalystParams::FBBuffCnt],
         params[CatalystParams::FBPoisCnt]},
        {}, [this]() { return calcFbCntEffect(); }));

    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[CatalystParams::Magic], params[CatalystParams::Capacity]}, {},
        [this]() { drawMagic(); }));

    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[CatalystParams::FBRegCnt], params[CatalystParams::FBBuffCnt],
         params[CatalystParams::FBPoisCnt]},
        {}, [this]() { drawFbCounts(); }));

    mParamSubs.push_back(ParameterSystem::subscribe(
        {},
        {states[State::BoughtPowerWizard], states[State::BoughtPoisonWizard]},
        [this, states]() {
            std::vector<RenderTextureCPtr> imgs = {
                IconSystem::Get(WizardDefs::FB_IMG)};
            if (states[State::BoughtPowerWizard].get()) {
                imgs.push_back(IconSystem::Get(WizardDefs::FB_BUFFED_IMG));
            }
            if (states[State::BoughtPoisonWizard].get()) {
                imgs.push_back(IconSystem::Get(WizardDefs::FB_POISON_IMG));
            }
            mFbCntText->setImgs(imgs);

            drawFbCounts();
        }));

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
            auto cnt = params[CatalystParams::FBBuffCnt];
            cnt.set(cnt.get() + 1);
        }
        if (poisoned) {
            auto cnt = params[CatalystParams::FBPoisCnt];
            cnt.set(cnt.get() + 1);
        }
    } else {
        auto cnt = params[CatalystParams::FBRegCnt];
        cnt.set(cnt.get() + 1);
    }
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
    mFbCntRender.setDest(Rect(mPos->rect.x(), mMagicRender.getDest().y2(),
                              mPos->rect.w(), FONT.h));
    tex.draw(mFbCntRender);
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
    return (params[CatalystParams::FBRegCnt].get() + 1) *
           ((params[CatalystParams::FBPoisCnt].get() + 1).sqrt() ^
            (params[CatalystParams::FBBuffCnt].get() + 10).logTen());
}

void Catalyst::drawMagic() {
    ParameterSystem::Params<CATALYST> params;
    std::stringstream ss;
    ss << "{i} " << params[CatalystParams::Magic].get() << "/{b}"
       << params[CatalystParams::Capacity].get();
    mMagicText->setText(ss.str(), mPos->rect.W());
}

void Catalyst::drawFbCounts() {
    ParameterSystem::Params<CATALYST> params;
    ParameterSystem::States states;

    std::stringstream ss;
    ss << params[CatalystParams::FBRegCnt].get() << "{i}";
    if (states[State::BoughtPowerWizard].get()) {
        ss << "\n" << params[CatalystParams::FBBuffCnt].get() << "{i}";
    }
    if (states[State::BoughtPoisonWizard].get()) {
        ss << "\n" << params[CatalystParams::FBPoisCnt].get() << "{i}";
    }
    mFbCntText->setText(ss.str(), mPos->rect.W());
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
