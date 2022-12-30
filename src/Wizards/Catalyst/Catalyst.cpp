#include "Catalyst.h"

namespace Catalyst {
// Catalyst
Catalyst::Catalyst() : WizardBase(CATALYST) {
    mPos->elevation = Elevation::CATALYST;
}

void Catalyst::init() {
    mMessages = ComponentFactory<MessageHandler>::New(FONT);

    Params params;
    mImg.set(Constants::IMG());
    mImg.setDest(IMG_RECT);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);

    mMagicText->setFont(FONT);
    mMagicText->setImgs({MoneyIcons::GetMoneyIcon(params[Param::Magic])});
    mMagicRender.set(mMagicText);
    mMagicRender.setFit(RenderData::FitMode::Texture);
    mMagicRender.setFitAlign(Rect::Align::CENTER, Rect::Align::TOP_LEFT);

    mRange = CircleShape(PURPLE).setDashed(50);

    setPos(mPos->rect.cX(), mPos->rect.cY());

    WizardBase::init();
}
void Catalyst::setSubscriptions() {
    mWizFireballSub = Wizard::FireballList::GetHitObservable()->subscribe(
        [this](const Wizard::Fireball& fb) { onWizFireballHit(fb); },
        [this](const Wizard::Fireball& fb) {
            return Wizard::Fireball::filter(fb, mId);
        },
        mPos);
    mPoisFireballSub =
        PoisonWizard::FireballList::GetHitObservable()->subscribe(
            [this](const PoisonWizard::Fireball& fb) { onPoisFireballHit(fb); },
            [this](const PoisonWizard::Fireball& fb) {
                return PoisonWizard::Fireball::filter(fb, mId);
            },
            mPos);
    mMagicSub = WizardSystem::GetCatalystMagicObservable()->subscribe(
        [this](const Number& amnt) { onMagic(amnt); });
    attachSubToVisibility(mWizFireballSub);
}
void Catalyst::setUpgrades() {
    Params params;
    Crystal::Params cryParams;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(WIZ_IMGS.at(mId));
    dUp->setEffects(
        {params[Param::MagicEffect], params[Param::FBCntEffect],
         params[Param::Range]},
        {}, []() -> TextUpdateData {
            Params params;

            std::stringstream ss;
            ss << "{i} Multiplier: "
               << UpgradeDefaults::MultiplicativeEffectText(
                      params[Param::MagicEffect].get())
               << "\n{i} Multiplier: "
               << UpgradeDefaults::MultiplicativeEffectText(
                      params[Param::FBCntEffect].get())
               << "\nRange: " << params[Param::Range].get();
            return {ss.str(),
                    {MoneyIcons::GetMoneyIcon(UpgradeDefaults::CATALYST_MAGIC),
                     IconSystem::Get(Wizard::Constants::FB_IMG())}};
        });
    mMagicEffectDisplay = mUpgrades->subscribe(dUp);

    // Boost catalyst gain
    UpgradePtr up = std::make_shared<Upgrade>(params[Param::GainUp1Lvl], 10);
    up->setImage("");
    up->setDescription(
        {"Multiplies {i} gain *2", {IconSystem::Get(Constants::IMG())}});
    up->setCost(UpgradeDefaults::CRYSTAL_SHARDS, params[Param::GainUp1Cost]);
    up->setEffects(params[Param::GainUp1],
                   UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(params[Param::GainUp1Cost].subscribeTo(
        up->level(), [](const Number& lvl) { return 10 * (2 ^ lvl); }));
    mParamSubs.push_back(params[Param::GainUp1].subscribeTo(
        up->level(), [](const Number& lvl) { return 2 ^ lvl; }));
    mGainUp1 = mUpgrades->subscribe(up);

    // Boost catalyst gain 2
    up = std::make_shared<Upgrade>(params[Param::GainUp2Lvl], 5);
    up->setImage("");
    up->setDescription(
        {"Improves {i} gain formula", {IconSystem::Get(Constants::IMG())}});
    up->setCost(UpgradeDefaults::CRYSTAL_SHARDS, params[Param::GainUp2Cost]);
    up->setEffects(
        params[Param::GainUp2], [](const Number& effect) -> TextUpdateData {
            return {"log(x) -> x" + UpgradeDefaults::PowerEffectText(effect)};
        });
    mParamSubs.push_back(params[Param::GainUp2Cost].subscribeTo(
        up->level(),
        [](const Number& lvl) { return Number(1, 4) * (5 ^ lvl); }));
    mParamSubs.push_back(params[Param::GainUp2].subscribeTo(
        up->level(), [](const Number& lvl) { return lvl / 10; }));
    mGainUp2 = mUpgrades->subscribe(up);

    // Boost shard gain
    UnlockablePtr uUp =
        std::make_shared<Unlockable>(params[Param::BoughtShardMult]);
    uUp->setImage("");
    uUp->setDescription(
        {"{i} boosts {i} gain",
         {MoneyIcons::GetMoneyIcon(UpgradeDefaults::CATALYST_MAGIC),
          MoneyIcons::GetMoneyIcon(UpgradeDefaults::CRYSTAL_SHARDS)}});
    uUp->setCost(UpgradeDefaults::CRYSTAL_SHARDS,
                 params[Param::ShardGainUpCost]);
    uUp->setEffects(params[Param::ShardGainUp],
                    UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(params[Param::ShardGainUp].subscribeTo(
        {params[Param::ShardMultUp]}, {uUp->level()}, [params]() -> Number {
            if (!params[Param::BoughtShardMult].get()) {
                return 1;
            }
            return params[Param::Magic].get().sqrtCopy() *
                   params[Param::ShardMultUp].get();
        }));
    mShardUp = mUpgrades->subscribe(uUp);

    // Catalyst multiplier upgrade
    uUp = std::make_shared<Unlockable>(params[Param::BoughtMultUp]);
    uUp->setImage("");
    uUp->setDescription(
        {"{i} multiplier to {i},{i} is increased based on log({i})",
         {
             IconSystem::Get(Constants::IMG()),
             MoneyIcons::GetMoneyIcon(UpgradeDefaults::CRYSTAL_MAGIC),
             MoneyIcons::GetMoneyIcon(UpgradeDefaults::CRYSTAL_SHARDS),
             MoneyIcons::GetMoneyIcon(UpgradeDefaults::CATALYST_MAGIC),
         }});
    uUp->setCost(UpgradeDefaults::CRYSTAL_SHARDS, params[Param::MultUpCost]);
    uUp->setEffects({params[Param::CatMultUp], params[Param::ShardMultUp]}, {},
                    [params]() -> TextUpdateData {
                        return {UpgradeDefaults::PowerEffectText(
                                    params[Param::CatMultUp].get()) +
                                ", " +
                                UpgradeDefaults::MultiplicativeEffectText(
                                    params[Param::ShardMultUp].get())};
                    });
    mParamSubs.push_back(params[Param::CatMultUp].subscribeTo(
        {params[Param::Magic]}, {uUp->level()}, [params]() {
            return params[Param::BoughtMultUp].get()
                       ? (params[Param::Magic].get() + 10).logTen() / 2
                       : 1;
        }));
    mParamSubs.push_back(params[Param::ShardMultUp].subscribeTo(
        {params[Param::Magic]}, {uUp->level()}, [params]() {
            return params[Param::BoughtMultUp].get()
                       ? (params[Param::Magic].get() + 10).logTen() ^ 2
                       : 1;
        }));
    mMultUp = mUpgrades->subscribe(uUp);

    // Zap range
    up = std::make_shared<Upgrade>(params[Param::RangeUpLvl], 5);
    up->setImage("");
    up->setDescription({"Increase range of fireball boost"});
    up->setCost(UpgradeDefaults::CRYSTAL_SHARDS, params[Param::RangeUpCost]);
    up->setEffects(params[Param::RangeUp],
                   UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(params[Param::RangeUpCost].subscribeTo(
        up->level(), [](const Number& lvl) { return 5 * (2 ^ lvl); }));
    mParamSubs.push_back(params[Param::RangeUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1.1 ^ lvl; }));
    mRangeUp = mUpgrades->subscribe(up);

    // Num zappers
    up = std::make_shared<Upgrade>(params[Param::ZapperUpLvl], 4);
    up->setImage("");
    up->setDescription({"Increases number and charge rate of zaps"});
    up->setCost(UpgradeDefaults::CRYSTAL_SHARDS, params[Param::ZapperUpCost]);
    up->setEffects(params[Param::ZapperUp], UpgradeDefaults::NoEffect);
    mParamSubs.push_back(params[Param::ZapperUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1 + lvl; }));
    mParamSubs.push_back(params[Param::ZapperUpCost].subscribeTo(
        up->level(), [](const Number& lvl) { return lvl * (lvl + 1); }));
    mZapperCntUp = mUpgrades->subscribe(up);

    // Zap count
    up = std::make_shared<Upgrade>(params[Param::ZapCntUpLvl], 3);
    up->setImage("");
    up->setDescription({"Increases the number of times each {i} can be zapped",
                        {IconSystem::Get(Wizard::Constants::FB_IMG())}});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC, params[Param::ZapCntUpCost]);
    up->setEffects(params[Param::ZapCntUp], UpgradeDefaults::NoEffect);
    mParamSubs.push_back(params[Param::ZapCntUpCost].subscribeTo(
        up->level(), [](const Number& lvl) { return 10 ^ (10 * (lvl + 1)); }));
    mParamSubs.push_back(params[Param::ZapCntUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1 + lvl; }));
    mZapCntUp = mUpgrades->subscribe(up);

    // Fireball count upgrade
    up = std::make_shared<Upgrade>(params[Param::FBCntLvl],
                                   params[Param::FBCntMaxLvl]);
    up->setImage("");
    mParamSubs.push_back(params[Param::FBCntMaxLvl].subscribeTo(
        {},
        {cryParams[Crystal::Param::BoughtPoisonWizard],
         cryParams[Crystal::Param::BoughtRobotWizard]},
        [cryParams]() {
            return cryParams[Crystal::Param::BoughtRobotWizard].get()    ? 3
                   : cryParams[Crystal::Param::BoughtPoisonWizard].get() ? 2
                                                                         : 1;
        }));
    mParamSubs.push_back(params[Param::FBCntUpCost].subscribeTo(
        up->level(), [up, params](const Number& lvl) {
            up->setCost(Constants::FB_CNT_TYPES.at(lvl.toInt()),
                        params[Param::FBCntUpCost]);
            switch (lvl.toInt()) {
                case Constants::FBCntUpBuyType::Reg:
                    return 1000;
                case Constants::FBCntUpBuyType::Pow:
                    return 100;
                case Constants::FBCntUpBuyType::Poi:
                    return 50;
            };
            return 0;
        }));
    mParamSubs.push_back(ParameterSystem::subscribe(
        {Constants::REG_FB_CNT, Constants::POW_FB_CNT, Constants::POI_FB_CNT,
         up->level()},
        {}, [up]() {
            int lvl = up->level().get().toInt();
            std::stringstream ss;
            for (int i = 0; i < lvl; i++) {
                if (i > 0) {
                    ss << " | ";
                }
                ss << Constants::FB_CNT_TYPES.at(i + 1).get() << "{i}";
            }
            up->setEffectText(ss.str());
        }));
    mParamSubs.push_back(ParameterSystem::subscribe(
        {up->level(), params[Param::FBCntMaxLvl]}, {}, [up, params]() {
            int lvl = up->level().get().toInt();
            int maxLvl = params[Param::FBCntMaxLvl].get().toInt();

            std::vector<RenderTextureCPtr> imgs;
            std::stringstream desc_str;
            desc_str << "Hitting {i} with {i}";
            for (int i = 0; i < lvl; i++) {
                if (i < maxLvl - 1) {
                    desc_str << ",{i}";
                }
                imgs.push_back(MoneyIcons::GetMoneyIcon(
                    Constants::FB_CNT_TYPES.at(i + 1)));
            }
            desc_str << " boosts {i} power";
            up->setEffectImgs(imgs);

            imgs.insert(imgs.begin(), IconSystem::Get(Constants::IMG()));
            if (lvl < maxLvl) {
                imgs.push_back(MoneyIcons::GetMoneyIcon(
                    Constants::FB_CNT_TYPES.at(lvl + 1)));
            }
            imgs.push_back(IconSystem::Get(Wizard::Constants::IMG()));

            up->setDescription({desc_str.str(), imgs});
        }));
    mFbCountUp = mUpgrades->subscribe(up);
}
void Catalyst::setParamTriggers() {
    Params params;
    Crystal::Params cryParams;

    mParamSubs.push_back(params[Param::MagicEffect].subscribeTo(
        {params[Param::Magic], params[Param::CatMultUp]}, {},
        [this]() { return calcMagicEffect(); }));

    mParamSubs.push_back(params[Param::Capacity].subscribeTo(
        cryParams[Crystal::Param::BestMagic],
        [](const Number& magic) { return (magic ^ 0.2) / 10; }));

    mParamSubs.push_back(params[Param::Range].subscribeTo(
        {params[Param::BaseRange], params[Param::RangeUp]}, {},
        [this]() { return calcRange(); }));
    mParamSubs.push_back(params[Param::FBCntEffect].subscribeTo(
        {Constants::REG_FB_CNT, Constants::POW_FB_CNT, Constants::POI_FB_CNT,
         params[Param::FBCntLvl]},
        {}, [this]() { return calcFbCntEffect(); }));

    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[Param::Magic], params[Param::Capacity]}, {},
        [this]() { drawMagic(); }));

    mParamSubs.push_back(cryParams[Crystal::Param::BoughtCatalyst].subscribe(
        [this](bool bought) {
            WizardSystem::GetHideObservable()->next(mId, !bought);
        }));

    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[Param::Range]}, {}, [this]() { updateRange(); }));

    // Upgrade unlock constraints
    mParamSubs.push_back(
        params[Param::GainUp1Lvl].subscribe([this](const Number& lvl) {
            mGainUp2->setActive(mGainUp1->get<UpgradeList::DATA>()->status() ==
                                Upgrade::BOUGHT);
        }));
    mParamSubs.push_back(
        cryParams[Crystal::Param::BoughtPoisonWizard].subscribe(
            [this](bool bought) {
                mZapCntUp->setActive(bought);
                mZapperCntUp->setActive(bought);
            }));
    mParamSubs.push_back(params[Param::BoughtShardMult].subscribe(
        [this](bool bought) { mMultUp->setActive(bought); }));
}

void Catalyst::onWizFireballHit(const Wizard::Fireball& fireball) {
    WizardSystem::GetCatalystMagicObservable()->next(fireball);

    Params params;
    bool buffed = fireball.isBoosted(), poisoned = fireball.isPoisoned();
    if (buffed || poisoned) {
        if (buffed) {
            auto cnt = Constants::POW_FB_CNT;
            cnt.set(cnt.get() + 1);
        }
        if (poisoned) {
            auto cnt = Constants::POI_FB_CNT;
            cnt.set(cnt.get() + 1);
        }
    } else {
        auto cnt = Constants::REG_FB_CNT;
        cnt.set(cnt.get() + 1);
    }
}
void Catalyst::onPoisFireballHit(const PoisonWizard::Fireball& fireball) {
    auto poisCnt = Params::get(Param::RingPoisCnt);
    Number poisCntUp =
        PoisonWizard::Params::get(PoisonWizard::Param::CatPoisCntUp).get();

    poisCnt.set(poisCnt.get() + poisCntUp);
}

void Catalyst::onMagic(const Number& amnt) {
    Params params;
    auto magic = params[Param::Magic];
    magic.set(max(0, min(magic.get() + amnt, params[Param::Capacity].get())));
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
    Params params;

    return (params[Param::Magic].get() + 1) ^
           (.5 * params[Param::CatMultUp].get());
}

Number Catalyst::calcRange() {
    Params params;
    return params[Param::BaseRange].get() * params[Param::RangeUp].get();
}

Number Catalyst::calcFbCntEffect() {
    Params params;
    Number reg_effect = 1, pow_effect = 1, poi_effect = 1;
    int lvl = params[Param::FBCntLvl].get().toInt();
    if (lvl > Constants::FBCntUpBuyType::Reg) {
        reg_effect = Constants::REG_FB_CNT.get() + 1;
        if (lvl > Constants::FBCntUpBuyType::Poi) {
            poi_effect = (Constants::POI_FB_CNT.get() + 1).sqrt();
            if (lvl > Constants::FBCntUpBuyType::Pow) {
                pow_effect = (Constants::POW_FB_CNT.get() + 10).logTen();
            }
        }
    }
    return reg_effect * ((poi_effect * pow_effect) ^ pow_effect);
}

void Catalyst::drawMagic() {
    Params params;
    std::stringstream ss;
    ss << "{i} " << params[Param::Magic].get() << "/{b}"
       << params[Param::Capacity].get();
    mMagicText->setText(ss.str(), mPos->rect.W());
}

void Catalyst::updateRange() {
    float half = fmaxf(mPos->rect.halfH(), mPos->rect.halfW());
    auto range = Params::get(Param::Range);
    mRange.setRadius((int)(half * range.get().toFloat()), ceilf(half / 100),
                     true);
    Ring::GetHitObservable()->setPos(mRange.get());
}

void Catalyst::setPos(float x, float y) {
    WizardBase::setPos(x, y);

    mRange.setCenter({mPos->rect.CX(), mPos->rect.CY()});
    Ring::GetHitObservable()->setPos(mRange.get());
}
}  // namespace Catalyst
