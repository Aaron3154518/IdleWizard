#include "CrystalDefs.h"

namespace CrystalDefs {
const Number T1_COST1 = 500, T1_COST2 = 5e4;
const SDL_Color MSG_COLOR{200, 0, 175, 255};

const AnimationData IMG{"res/wizards/crystal_ss.png", 13, 100};

void setDefaults() {
    using WizardSystem::Event;

    ParameterSystem::Params<CRYSTAL> params;

    // Default 0
    params[CrystalParams::Magic]->init(Number(1, 10), Event::ResetT1);
    params[CrystalParams::Shards]->init(0, Event::ResetT2);

    params[CrystalParams::WizardCntUpCost]->init(Number(2, 3));
    params[CrystalParams::GlowUpCost]->init(Number(1, 5));
    params[CrystalParams::CatalystCost]->init(1);

    ParameterSystem::States states;

    states[State::ResetT1]->init(false);
    states[State::BoughtCrysWizCntUp]->init(false, Event::ResetT1);
    states[State::BoughtCrysGlowUp]->init(false, Event::ResetT1);
    states[State::BoughtPowerWizard]->init(false, Event::ResetT1);
    states[State::BoughtTimeWizard]->init(false, Event::ResetT1);
    states[State::BoughtCatalyst]->init(false, Event::ResetT2);
}

RenderDataWPtr GetIcon() {
    static RenderDataPtr ICON;
    static TimerObservable::SubscriptionPtr ANIM_SUB;
    if (!ICON) {
        ICON = std::make_shared<RenderData>();
        ICON->set(IMG);
        ANIM_SUB =
            ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
                [](Timer& t) {
                    ICON->nextFrame();
                    return true;
                },
                Timer(IMG.frame_ms));
    }

    return ICON;
}

}  // namespace CrystalDefs
