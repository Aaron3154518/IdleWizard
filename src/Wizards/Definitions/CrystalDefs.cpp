#include "CrystalDefs.h"

namespace CrystalDefs {
const Number T1_COST1 = 500, T1_COST2 = 1e6;
const SDL_Color MSG_COLOR{200, 0, 175, 255}, GLOW_MSG_COLOR{175, 100, 200, 255},
    POISON_MSG_COLOR{105, 130, 25, 255};

const std::string WIZ_CNT_UP_IMG = "res/upgrades/wizard_count_upgrade.png",
                  CRYS_GLOW_UP_IMG = "res/upgrades/crystal_glow_upgrade.png",
                  FRACTURE_IMG = "res/upgrades/fracture.png";

const AnimationData IMG{"res/wizards/crystal_ss.png", 13, 100},
    GLOW_EFFECT_IMG{"res/wizards/crystal_glow_effect_ss.png", 4, 150},
    GLOW_FINISH_IMG{"res/wizards/crystal_glow_finish_ss.png", 9, 125};

void setDefaults() {
    using WizardSystem::Event;

    ParameterSystem::Params<CRYSTAL> params;

    // Default 0
    params[CrystalParams::Magic]->init(0, Event::ResetT1);
    params[CrystalParams::Shards]->init(0, Event::ResetT2);

    params[CrystalParams::WizardCntUpCost]->init(Number(1, 4));
    params[CrystalParams::GlowUpCost]->init(Number(1, 11));
    params[CrystalParams::CatalystCost]->init(1);
    params[CrystalParams::PoisonWizCost]->init(100);
    params[CrystalParams::RobotCost]->init(1000);
    params[CrystalParams::T1ResetCost]->init(Number(1, 15));

    ParameterSystem::States states;

    states[State::ResetT1]->init(false);
    states[State::CrysGlowActive]->init(false, Event::ResetT1);
    states[State::CrysPoisoned]->init(false, Event::ResetT1);
    states[State::BoughtCrysWizCntUp]->init(false, Event::ResetT1);
    states[State::BoughtCrysGlowUp]->init(false, Event::ResetT1);
    states[State::BoughtPowerWizard]->init(false, Event::ResetT1);
    states[State::BoughtTimeWizard]->init(false, Event::ResetT1);
    states[State::BoughtCatalyst]->init(false, Event::ResetT2);
    states[State::BoughtPoisonWizard]->init(false, Event::ResetT2);
    states[State::BoughtRobotWizard]->init(false, Event::ResetT2);
}
}  // namespace CrystalDefs
