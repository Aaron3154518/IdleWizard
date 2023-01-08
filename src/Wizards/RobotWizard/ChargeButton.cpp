#include "ChargeButton.h"

namespace RobotWizard {
// ChargeButton
ChargeButton::ChargeButton()
    : mPos(std::make_shared<UIComponent>(Rect(), Elevation::WIZARDS)) {}

void ChargeButton::init() {
    setImage("");
    setDescription({"Charge {i} with 10% of your current {i}",
                    {IconSystem::Get(RobotWizard::Constants::IMG()),
                     MoneyIcons::Get(UpgradeDefaults::CRYSTAL_SHARDS)}});
    setEffects(UpgradeDefaults::CRYSTAL_SHARDS,
               [this](const Number& val) -> TextUpdateData {
                   return {"+{i}" + getGainAmnt().toString(),
                           {MoneyIcons::Get(UpgradeDefaults::CRYSTAL_SHARDS)}};
               });

    mMouseSub = EventServices::GetMouseObservable()->subscribeLeftClick(
        [this](Event::MouseButton b, bool clicked) { onClick(b, clicked); },
        mPos);
    mHoverSub = EventServices::GetHoverObservable()->subscribe(
        [this]() { onMouseEnter(); }, [](SDL_Point p) {},
        [this]() { onMouseLeave(); }, mPos);
    mRobotPosSub = WizardSystem::GetWizardPosObservable()->subscribe(
        [this](const Rect& r) { onRobotPos(r); }, ROBOT_WIZARD);
}

void ChargeButton::onRender(SDL_Renderer* r) {
    TextureBuilder tex;
    drawIcon(tex, mPos->rect);

    RectShape rect = RectShape().set(mPos->rect, 2, true);
    tex.draw(rect);
}
void ChargeButton::onClick(Event::MouseButton b, bool clicked) {
    if (clicked) {
        auto shards = UpgradeDefaults::CRYSTAL_SHARDS;
        auto roboShards = UpgradeDefaults::ROBOT_SHARDS;
        Number amnt = getGainAmnt();
        shards.set(shards.get() - amnt);
        roboShards.set(roboShards.get() + amnt);
    }
}
void ChargeButton::onMouseEnter() {
    mDescRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [this](SDL_Renderer* r) {
                drawDescription(
                    TextureBuilder(),
                    mPos->rect.getPos(Rect::CENTER, Rect::BOT_RIGHT));
            },
            std::make_shared<UIComponent>(Rect(), Elevation::WIZARD_OVERLAYS));
}
void ChargeButton::onMouseLeave() { mDescRenderSub.reset(); }
void ChargeButton::onRobotPos(const Rect& r) {
    float w = fminf(r.w(), r.h()) / 3;
    mPos->rect.setDim(w, w);
    mPos->rect.setPos(r.cX(), r.y(), Rect::Align::CENTER,
                      Rect::Align::BOT_RIGHT);
}

Number ChargeButton::getGainAmnt() {
    Number shards = UpgradeDefaults::CRYSTAL_SHARDS.get();
    if (shards < 1) {
        return 0;
    }

    if (shards < 10) {
        return 1;
    }

    return (shards / 10 + .5).floorNum();
}

bool ChargeButton::isHidden() const { return !mPos->visible; }
void ChargeButton::setHidden(bool hidden) { mPos->visible = !hidden; }
}  // namespace RobotWizard
