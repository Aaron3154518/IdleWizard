#include "FractureButton.h"

namespace Crystal {
// FractureButton
FractureButton::FractureButton()
    : mPos(std::make_shared<UIComponent>(Rect(), Elevation::WIZARDS)) {
    setImage(Crystal::Constants::FRACTURE_IMG);
    setDescription({"Blast {i} with all your strength\nDestroys your wizards",
                    {IconSystem::Get(Crystal::Constants::IMG())}});
    setEffects(ParameterSystem::Param<CRYSTAL>(CrystalParams::ShardGain),
               [](const Number& gain) -> TextUpdateData {
                   return {
                       "+{i}" + gain.toString(),
                       {MoneyIcons::GetMoneyIcon(UpgradeDefaults::CRYSTAL_SHARDS)}};
               });
}

void FractureButton::init() {
    mMouseSub = ServiceSystem::Get<MouseService, MouseObservable>()->subscribe(
        [this](Event::MouseButton b, bool clicked) { onClick(b, clicked); },
        mPos);
    mHoverSub = ServiceSystem::Get<HoverService, HoverObservable>()->subscribe(
        [this]() { onMouseEnter(); }, [](SDL_Point p) {},
        [this]() { onMouseLeave(); }, mPos);
    mCrysPosSub = WizardSystem::GetWizardPosObservable()->subscribe(
        [this](const Rect& r) { onCrystalPos(r); }, CRYSTAL);
}

void FractureButton::onRender(SDL_Renderer* r) {
    TextureBuilder tex;
    drawIcon(tex, mPos->rect);

    RectShape rect = RectShape().set(mPos->rect, 2, true);
    tex.draw(rect);
}
void FractureButton::onClick(Event::MouseButton b, bool clicked) {
    if (clicked) {
        auto shards = ParameterSystem::Param<CRYSTAL>(CrystalParams::Shards);
        auto shardGain =
            ParameterSystem::Param<CRYSTAL>(CrystalParams::ShardGain);
        shards.set(shards.get() + shardGain.get());

        auto resetT1 = ParameterSystem::Param(State::ResetT1);
        if (!resetT1.get()) {
            resetT1.set(true);
        }

        WizardSystem::GetWizardEventObservable()->next(
            WizardSystem::Event::ResetT1);
    }
}
void FractureButton::onMouseEnter() {
    mDescRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [this](SDL_Renderer* r) {
                drawDescription(
                    TextureBuilder(),
                    mPos->rect.getPos(Rect::CENTER, Rect::BOT_RIGHT));
            },
            std::make_shared<UIComponent>(Rect(), Elevation::WIZARD_OVERLAYS));
}
void FractureButton::onMouseLeave() { mDescRenderSub.reset(); }
void FractureButton::onCrystalPos(const Rect& r) {
    float w = fminf(r.w(), r.h()) / 3;
    mPos->rect.setDim(w, w);
    mPos->rect.setPos(r.cX(), r.y(), Rect::Align::CENTER,
                      Rect::Align::BOT_RIGHT);
}

bool FractureButton::isHidden() const { return !mPos->visible; }
void FractureButton::setHidden(bool hidden) { mPos->visible = !hidden; }
}  // namespace Crystal
