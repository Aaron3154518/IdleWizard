#include "Catalyst.h"

namespace Catalyst {
// Catalyst
Catalyst::Catalyst() : WizardBase(CATALYST) {
    mPos->elevation = Elevation::CATALYST;
}

void Catalyst::init() {
    mMessages = ComponentFactory<MessageHandler>::New(FONT);

    Catalyst::Params params;
    mImg.set(Constants::IMG());
    mImg.setDest(IMG_RECT);
    mPos->rect = mImg.getDest();
    WizardSystem::GetWizardImageObservable()->next(mId, mImg);

    mMagicText->setFont(FONT);
    mMagicText->setImgs(
        {MoneyIcons::GetMoneyIcon(params[Catalyst::Param::Magic])});
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
    Catalyst::Params params;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(WIZ_IMGS.at(mId));
    dUp->setEffects(
        {params[Catalyst::Param::MagicEffect],
         params[Catalyst::Param::FBCntEffect], params[Catalyst::Param::Range]},
        {}, []() -> TextUpdateData {
            Catalyst::Params params;

            std::stringstream ss;
            ss << "{i} Multiplier: "
               << UpgradeDefaults::MultiplicativeEffectText(
                      params[Catalyst::Param::MagicEffect].get())
               << "\n{i} Multiplier: "
               << UpgradeDefaults::MultiplicativeEffectText(
                      params[Catalyst::Param::FBCntEffect].get())
               << "\nRange: " << params[Catalyst::Param::Range].get();
            return {ss.str(),
                    {MoneyIcons::GetMoneyIcon(UpgradeDefaults::CATALYST_MAGIC),
                     IconSystem::Get(Wizard::Constants::FB_IMG())}};
        });
    mMagicEffectDisplay = mUpgrades->subscribe(dUp);

    // Boost catalyst gain
    UpgradePtr up =
        std::make_shared<Upgrade>(params[Catalyst::Param::GainUp1Lvl], 10);
    up->setImage("");
    up->setDescription(
        {"Multiplies {i} gain *2", {IconSystem::Get(Constants::IMG())}});
    up->setCost(UpgradeDefaults::CRYSTAL_SHARDS,
                params[Catalyst::Param::GainUp1Cost]);
    up->setEffects(params[Catalyst::Param::GainUp1],
                   UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(params[Catalyst::Param::GainUp1Cost].subscribeTo(
        up->level(), [](const Number& lvl) { return 10 * (2 ^ lvl); }));
    mParamSubs.push_back(params[Catalyst::Param::GainUp1].subscribeTo(
        up->level(), [](const Number& lvl) { return 2 ^ lvl; }));
    mGainUp1 = mUpgrades->subscribe(up);

    // Boost catalyst gain 2
    up = std::make_shared<Upgrade>(params[Catalyst::Param::GainUp2Lvl], 5);
    up->setImage("");
    up->setDescription(
        {"Improves {i} gain formula", {IconSystem::Get(Constants::IMG())}});
    up->setCost(UpgradeDefaults::CRYSTAL_SHARDS,
                params[Catalyst::Param::GainUp2Cost]);
    up->setEffects(
        params[Catalyst::Param::GainUp2],
        [](const Number& effect) -> TextUpdateData {
            return {"log(x) -> x" + UpgradeDefaults::PowerEffectText(effect)};
        });
    mParamSubs.push_back(params[Catalyst::Param::GainUp2Cost].subscribeTo(
        up->level(),
        [](const Number& lvl) { return Number(1, 4) * (5 ^ lvl); }));
    mParamSubs.push_back(params[Catalyst::Param::GainUp2].subscribeTo(
        up->level(), [](const Number& lvl) { return lvl / 10; }));
    mGainUp2 = mUpgrades->subscribe(up);

    // Boost shard gain
    UnlockablePtr uUp =
        std::make_shared<Unlockable>(states[State::BoughtCatShardMult]);
    uUp->setImage("");
    uUp->setDescription(
        {"{i} boosts {i} gain",
         {MoneyIcons::GetMoneyIcon(UpgradeDefaults::CATALYST_MAGIC),
          MoneyIcons::GetMoneyIcon(UpgradeDefaults::CRYSTAL_SHARDS)}});
    uUp->setCost(UpgradeDefaults::CRYSTAL_SHARDS,
                 params[Catalyst::Param::ShardGainUpCost]);
    uUp->setEffects(params[Catalyst::Param::ShardGainUp],
                    UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(params[Catalyst::Param::ShardGainUp].subscribeTo(
        {params[Catalyst::Param::ShardMultUp]}, {uUp->level()},
        [params]() -> Number {
            if (!ParameterSystem::Param(State::BoughtCatShardMult).get()) {
                return 1;
            }
            return params[Catalyst::Param::Magic].get().sqrtCopy() *
                   params[Catalyst::Param::ShardMultUp].get();
        }));
    mShardUp = mUpgrades->subscribe(uUp);

    // Catalyst multiplier upgrade
    uUp = std::make_shared<Unlockable>(states[State::BoughtMultUp]);
    uUp->setImage("");
    uUp->setDescription(
        {"{i} multiplier to {i},{i} is increased based on log({i})",
         {
             IconSystem::Get(Constants::IMG()),
             MoneyIcons::GetMoneyIcon(UpgradeDefaults::CRYSTAL_MAGIC),
             MoneyIcons::GetMoneyIcon(UpgradeDefaults::CRYSTAL_SHARDS),
             MoneyIcons::GetMoneyIcon(UpgradeDefaults::CATALYST_MAGIC),
         }});
    uUp->setCost(UpgradeDefaults::CRYSTAL_SHARDS,
                 params[Catalyst::Param::MultUpCost]);
    uUp->setEffects({params[Catalyst::Param::CatMultUp],
                     params[Catalyst::Param::ShardMultUp]},
                    {}, [params]() -> TextUpdateData {
                        return {UpgradeDefaults::PowerEffectText(
                                    params[Catalyst::Param::CatMultUp].get()) +
                                ", " +
                                UpgradeDefaults::MultiplicativeEffectText(
                                    params[Catalyst::Param::ShardMultUp].get())};
                    });
    mParamSubs.push_back(params[Catalyst::Param::CatMultUp].subscribeTo(
        {params[Catalyst::Param::Magic]}, {uUp->level()}, [params, states]() {
            return states[State::BoughtMultUp].get()
                       ? (params[Catalyst::Param::Magic].get() + 10).logTen() / 2
                       : 1;
        }));
    mParamSubs.push_back(params[Catalyst::Param::ShardMultUp].subscribeTo(
        {params[Catalyst::Param::Magic]}, {uUp->level()}, [params, states]() {
            return states[State::BoughtMultUp].get()
                       ? (params[Catalyst::Param::Magic].get() + 10).logTen() ^ 2
                       : 1;
        }));
    mMultUp = mUpgrades->subscribe(uUp);

    // Zap range
    up = std::make_shared<Upgrade>(params[Catalyst::Param::RangeUpLvl], 5);
    up->setImage("");
    up->setDescription({"Increase range of fireball boost"});
    up->setCost(UpgradeDefaults::CRYSTAL_SHARDS,
                params[Catalyst::Param::RangeUpCost]);
    up->setEffects(params[Catalyst::Param::RangeUp],
                   UpgradeDefaults::MultiplicativeEffect);
    mParamSubs.push_back(params[Catalyst::Param::RangeUpCost].subscribeTo(
        up->level(), [](const Number& lvl) { return 5 * (2 ^ lvl); }));
    mParamSubs.push_back(params[Catalyst::Param::RangeUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1.1 ^ lvl; }));
    mRangeUp = mUpgrades->subscribe(up);

    // Num zappers
    up = std::make_shared<Upgrade>(params[Catalyst::Param::ZapperUpLvl], 4);
    up->setImage("");
    up->setDescription({"Increases number and charge rate of zaps"});
    up->setCost(UpgradeDefaults::CRYSTAL_SHARDS,
                params[Catalyst::Param::ZapperUpCost]);
    up->setEffects(params[Catalyst::Param::ZapperUp], UpgradeDefaults::NoEffect);
    mParamSubs.push_back(params[Catalyst::Param::ZapperUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1 + lvl; }));
    mParamSubs.push_back(params[Catalyst::Param::ZapperUpCost].subscribeTo(
        up->level(), [](const Number& lvl) { return lvl * (lvl + 1); }));
    mZapperCntUp = mUpgrades->subscribe(up);

    // Zap count
    up = std::make_shared<Upgrade>(params[Catalyst::Param::ZapCntUpLvl], 3);
    up->setImage("");
    up->setDescription({"Increases the number of times each {i} can be zapped",
                        {IconSystem::Get(Wizard::Constants::FB_IMG())}});
    up->setCost(UpgradeDefaults::CRYSTAL_MAGIC,
                params[Catalyst::Param::ZapCntUpCost]);
    up->setEffects(params[Catalyst::Param::ZapCntUp], UpgradeDefaults::NoEffect);
    mParamSubs.push_back(params[Catalyst::Param::ZapCntUpCost].subscribeTo(
        up->level(), [](const Number& lvl) { return 10 ^ (10 * (lvl + 1)); }));
    mParamSubs.push_back(params[Catalyst::Param::ZapCntUp].subscribeTo(
        up->level(), [](const Number& lvl) { return 1 + lvl; }));
    mZapCntUp = mUpgrades->subscribe(up);

    // Fireball count upgrade
    up = std::make_shared<Upgrade>(params[Catalyst::Param::FBCntLvl],
                                   params[Catalyst::Param::FBCntMaxLvl]);
    up->setImage("");
    mParamSubs.push_back(params[Catalyst::Param::FBCntMaxLvl].subscribeTo(
        {},
        {states[Crystal::Param::BoughtPoisonWizard], states[Crystal::Param::BoughtRobotWizard]},
        [states]() {
            return states[Crystal::Param::BoughtRobotWizard].get()    ? 3
                   : states[Crystal::Param::BoughtPoisonWizard].get() ? 2
                                                             : 1;
        }));
    mParamSubs.push_back(params[Catalyst::Param::FBCntUpCost].subscribeTo(
        up->level(), [up, params](const Number& lvl) {
            up->setCost(Constants::FB_CNT_TYPES.at(lvl.toInt()),
                        params[Catalyst::Param::FBCntUpCost]);
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
        {up->level(), params[Catalyst::Param::FBCntMaxLvl]}, {}, [up, params]() {
            int lvl = up->level().get().toInt();
            int maxLvl = params[Catalyst::Param::FBCntMaxLvl].get().toInt();

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
    Catalyst::Params params;
    Crystal::Params cryParams;

    mParamSubs.push_back(params[Catalyst::Param::MagicEffect].subscribeTo(
        {params[Catalyst::Param::Magic], params[Catalyst::Param::CatMultUp]}, {},
        [this]() { return calcMagicEffect(); }));

    mParamSubs.push_back(params[Catalyst::Param::Capacity].subscribeTo(
        cryParams[Crystal::Param::BestMagic],
        [](const Number& magic) { return (magic ^ 0.2) / 10; }));

    mParamSubs.push_back(params[Catalyst::Param::Range].subscribeTo(
        {params[Catalyst::Param::BaseRange], params[Catalyst::Param::RangeUp]},
        {}, [this]() { return calcRange(); }));
    mParamSubs.push_back(params[Catalyst::Param::FBCntEffect].subscribeTo(
        {Constants::REG_FB_CNT, Constants::POW_FB_CNT, Constants::POI_FB_CNT,
         params[Catalyst::Param::FBCntLvl]},
        {}, [this]() { return calcFbCntEffect(); }));

    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[Catalyst::Param::Magic], params[Catalyst::Param::Capacity]}, {},
        [this]() { drawMagic(); }));

    mParamSubs.push_back(ParameterSystem::Param(Crystal::Param::BoughtCatalyst)
                             .subscribe([this](bool bought) {
                                 WizardSystem::GetHideObservable()->next(
                                     mId, !bought);
                             }));

    mParamSubs.push_back(ParameterSystem::subscribe(
        {params[Catalyst::Param::Range]}, {}, [this]() { updateRange(); }));

    // Upgrade unlock constraints
    mParamSubs.push_back(
        params[Catalyst::Param::GainUp1Lvl].subscribe([this](const Number& lvl) {
            mGainUp2->setActive(mGainUp1->get<UpgradeList::DATA>()->status() ==
                                Upgrade::BOUGHT);
        }));
    mParamSubs.push_back(
        states[Crystal::Param::BoughtPoisonWizard].subscribe([this](bool bought) {
            mZapCntUp->setActive(bought);
            mZapperCntUp->setActive(bought);
        }));
    mParamSubs.push_back(states[State::BoughtCatShardMult].subscribe(
        [this](bool bought) { mMultUp->setActive(bought); }));
}

void Catalyst::onWizFireballHit(const Wizard::Fireball& fireball) {
    WizardSystem::GetCatalystMagicObservable()->next(fireball);

    Catalyst::Params params;
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
    auto poisCnt =
        Catalyst::Params::get(Catalyst::Param::CatRingPoisCnt);
    Number poisCntUp =
        PoisonWizard::Params::get(PoisonWizard::Param::CatPoisCntUp)
            .get();

    poisCnt.set(poisCnt.get() + poisCntUp);
}

void Catalyst::onMagic(const Number& amnt) {
    Catalyst::Params params;
    auto magic = params[Catalyst::Param::Magic];
    magic.set(max(
        0, min(magic.get() + amnt, params[Catalyst::Param::Capacity].get())));
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
    Catalyst::Params params;

    return (params[Catalyst::Param::Magic].get() + 1) ^
           (.5 * params[Catalyst::Param::CatMultUp].get());
}

Number Catalyst::calcRange() {
    Catalyst::Params params;
    return params[Catalyst::Param::BaseRange].get() *
           params[Catalyst::Param::RangeUp].get();
}

Number Catalyst::calcFbCntEffect() {
    Catalyst::Params params;
    Number reg_effect = 1, pow_effect = 1, poi_effect = 1;
    int lvl = params[Catalyst::Param::FBCntLvl].get().toInt();
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
    Catalyst::Params params;
    std::stringstream ss;
    ss << "{i} " << params[Catalyst::Param::Magic].get() << "/{b}"
       << params[Catalyst::Param::Capacity].get();
    mMagicText->setText(ss.str(), mPos->rect.W());
}

void Catalyst::updateRange() {
    float half = fmaxf(mPos->rect.halfH(), mPos->rect.halfW());
    auto range = Catalyst::Params::get(Catalyst::Param::Range);
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
