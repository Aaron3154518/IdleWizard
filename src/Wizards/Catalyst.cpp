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
}
void Catalyst::setParamTriggers() {
    ParameterSystem::Params<CATALYST> params;

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

    mParamSubs.push_back(ParameterSystem::Param(State::BoughtCatalyst)
                             .subscribe([this](bool bought) {
                                 WizardSystem::GetHideObservable()->next(
                                     mId, !bought);
                             }));

    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[CatalystParams::Range]}, {}, [this]() { updateRange(); }));
}

void Catalyst::onWizFireballHit(const WizardFireball& fireball) {
    ParameterSystem::Params<CATALYST> params;
    auto magic = params[CatalystParams::Magic];
    Number gain = calcGain(fireball.getPower());
    magic.set(max(
        0, min(magic.get() + gain, params[CatalystParams::Capacity].get())));
    mMessages->addMessage(mPos->rect, "+" + gain.toString(), RED);

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

void Catalyst::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    TextureBuilder tex;

    tex.draw(mRange);

    mMessages->draw(tex);

    tex.draw(mMagicRender);
}

Number Catalyst::calcMagicEffect() {
    ParameterSystem::Params<CATALYST> params;
    return (params[CatalystParams::Magic].get() + 1).logTen() + 1;
}

Number Catalyst::calcRange() {
    ParameterSystem::Params<CATALYST> params;
    return params[CatalystParams::BaseRange].get() *
           params[CatalystParams::RangeUp].get();
}

Number Catalyst::calcGain(Number magic) {
    ParameterSystem::Params<CATALYST> params;
    ParameterSystem::Params<POISON_WIZARD> poiParams;

    if (poiParams[PoisonWizardParams::CatGainUp2Lvl].get() > 0) {
        magic ^= poiParams[PoisonWizardParams::CatGainUp2].get();
    } else {
        magic.logTen();
    }

    magic *= poiParams[PoisonWizardParams::CatGainUp1].get();

    return magic;
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
    std::vector<RenderTextureCPtr> imgs = {
        Money::GetMoneyIcon(params[CatalystParams::Magic]),
        IconSystem::Get(WizardDefs::FB_IMG)};

    ss << "{i} " << params[CatalystParams::Magic].get() << "/{b}"
       << params[CatalystParams::Capacity].get() << "\n"
       << params[CatalystParams::FBRegCnt].get() << "{i}";
    Number cnt = params[CatalystParams::FBBuffCnt].get();
    if (cnt > 0) {
        ss << " " << cnt << "{i}";
        imgs.push_back(IconSystem::Get(WizardDefs::FB_BUFFED_IMG));
    }
    cnt = params[CatalystParams::FBPoisCnt].get();
    if (cnt > 0) {
        ss << " " << cnt << "{i}";
        imgs.push_back(IconSystem::Get(WizardDefs::FB_POISON_IMG));
    }
    mMagicText->setText(ss.str(), mPos->rect.W());
    mMagicText->setImgs(imgs);
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

    mMagicRender.setDest(
        Rect(mPos->rect.x(), mPos->rect.y2(), mPos->rect.w(), FONT.h));

    mRange.setCenter({mPos->rect.CX(), mPos->rect.CY()});
    CatalystRing::GetHitObservable()->setPos(mRange.get());
}
