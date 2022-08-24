#include "Catalyst.h"

// Catalyst::FireballObservable
void Catalyst::HitObservable::setPos(const CircleData& pos) { mPos = pos; }

void Catalyst::HitObservable::init() {
    mTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) { return onTimer(t); }, Timer(1000));
}

bool Catalyst::HitObservable::onTimer(Timer& timer) {
    std::vector<SubscriptionWPtr> inRange;
    for (auto sub : *this) {
        auto& pos = sub->get<DATA>();
        float dx = mPos.c.x - pos->rect.cX(), dy = mPos.c.y - pos->rect.cY();
        float mag = sqrtf(dx * dx + dy * dy);
        if (mag < mPos.r2) {
            inRange.push_back(sub);
        }
    }

    if (inRange.empty()) {
        return true;
    }

    int idx = (int)(rDist(gen) * inRange.size());
    auto sub = inRange.at(idx).lock();
    if (sub) {
        ParameterSystem::Params<CATALYST> params;
        sub->get<FUNC>()(params[CatalystParams::MagicEffect].get());
        std::cerr << "Shoot fireball" << std::endl;
    }
    return true;
}

// Catalyst
void Catalyst::setDefaults() {
    using WizardSystem::ResetTier;

    ParameterSystem::Params<CATALYST> params;

    params[CatalystParams::Magic]->init(0);
    params[CatalystParams::Capacity]->init(100);
    params[CatalystParams::BaseRange]->init(1.25);

    params[CatalystParams::RangeUpLvl]->init(ResetTier::T2);
}

Catalyst::Catalyst() : WizardBase(CATALYST) {
    mPos->elevation = Elevation::CATALYST;
}

void Catalyst::init() {
    mMagicText.tData.font = AssetManager::getFont(FONT);

    mRange.color = PURPLE;
    mRange.setDashed(50);

    WizardBase::init();
}
void Catalyst::setSubscriptions() {
    mFireballSub =
        ServiceSystem::Get<FireballService, Fireball::HitObservable>()
            ->subscribe([this](const Fireball& f) { onFireballHit(f); }, mId);
    attachSubToVisibility(mFireballSub);
}
void Catalyst::setUpgrades() {
    ParameterSystem::Params<CATALYST> params;

    // Power Display
    DisplayPtr dUp = std::make_shared<Display>();
    dUp->setImage(WIZ_IMGS.at(mId));
    dUp->setEffects(
        {params[CatalystParams::MagicEffect], params[CatalystParams::Range]},
        {}, []() {
            ParameterSystem::Params<CATALYST> params;

            std::stringstream ss;
            ss << "Multiplier: "
               << Upgrade::Defaults::MultiplicativeEffect(
                      params[CatalystParams::MagicEffect].get())
               << "\nRange: " << params[CatalystParams::Range].get();
            return ss.str();
        });
    mMagicEffectDisplay = mUpgrades->subscribe(dUp);

    UpgradePtr up =
        std::make_shared<Upgrade>(params[CatalystParams::RangeUpLvl], 5);
    up->setImage("");
    up->setDescription("Increase range of fireball boost");
    up->setCost(Upgrade::Defaults::CRYSTAL_SHARDS,
                params[CatalystParams::RangeUpCost],
                [](const Number& lvl) { return 5 * (2 ^ lvl); });
    up->setEffect(
        params[CatalystParams::RangeUp],
        [](const Number& lvl) { return 1.1 ^ lvl; },
        Upgrade::Defaults::MultiplicativeEffect);
    mRangeUp = mUpgrades->subscribe(up);
}
void Catalyst::setParamTriggers() {
    ParameterSystem::Params<CATALYST> params;

    mParamSubs.push_back(params[CatalystParams::MagicEffect].subscribeTo(
        {params[CatalystParams::Magic]}, {},
        [this]() { return calcMagicEffect(); }));

    mParamSubs.push_back(params[CatalystParams::Range].subscribeTo(
        {params[CatalystParams::BaseRange], params[CatalystParams::RangeUp]},
        {}, [this]() { return calcRange(); }));

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

void Catalyst::onFireballHit(const Fireball& fireball) {
    switch (fireball.getSourceId()) {
        case WIZARD:
            ParameterSystem::Params<CATALYST> params;
            auto magic = params[CatalystParams::Magic];
            magic.set(max(min(magic.get() + fireball.getValue(),
                              params[CatalystParams::Capacity].get()),
                          0));
            break;
    }
}

void Catalyst::onRender(SDL_Renderer* r) {
    WizardBase::onRender(r);

    TextureBuilder tex;

    tex.draw(mRange);

    mMagicText.dest = Rect(mPos->rect.x(), mPos->rect.y2(), mPos->rect.w(), 0);
    mMagicText.dest.setHeight(FONT.h, Rect::Align::TOP_LEFT);
    mMagicText.shrinkToTexture();
    tex.draw(mMagicText);
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

void Catalyst::drawMagic() {
    ParameterSystem::Params<CATALYST> params;
    mMagicText.tData.text = params[CatalystParams::Magic].get().toString() +
                            "/" +
                            params[CatalystParams::Capacity].get().toString();
    mMagicText.renderText();
}

void Catalyst::updateRange() {
    float half = fmaxf(mPos->rect.halfH(), mPos->rect.halfW());
    auto range = ParameterSystem::Param<CATALYST>(CatalystParams::Range);
    mRange.setRadius((int)(half * range.get().toFloat()), ceilf(half / 100),
                     true);
    ServiceSystem::Get<CatalystService, HitObservable>()->setPos(mRange.get());
}

void Catalyst::setPos(float x, float y) {
    WizardBase::setPos(x, y);
    mRange.setCenter({mPos->rect.CX(), mPos->rect.CY()});
    ServiceSystem::Get<CatalystService, HitObservable>()->setPos(mRange.get());
}
