#include "CrystalConstants.h"

namespace Crystal {
namespace Constants {
const Number T1_COST1 = 500, T1_COST2 = 1e6;
const SDL_Color MSG_COLOR{200, 0, 175, 255}, GLOW_MSG_COLOR{175, 100, 200, 255},
    POISON_MSG_COLOR{105, 130, 25, 255};

const std::string WIZ_CNT_UP_IMG = "res/upgrades/wizard_count_upgrade.png",
                  CRYS_GLOW_UP_IMG = "res/upgrades/crystal_glow_upgrade.png",
                  FRACTURE_IMG = "res/upgrades/fracture.png";

const AnimationData& IMG() {
    const static AnimationData IMG{"res/wizards/crystal_ss.png", 13, 100};
    return IMG;
}
const AnimationData& GLOW_EFFECT_IMG() {
    const static AnimationData GLOW_EFFECT_IMG{
        "res/wizards/crystal_glow_effect_ss.png", 4, 150};
    return GLOW_EFFECT_IMG;
}
const AnimationData& GLOW_FINISH_IMG() {
    const static AnimationData GLOW_FINISH_IMG{
        "res/wizards/crystal_glow_finish_ss.png", 9, 125};
    return GLOW_FINISH_IMG;
}

void setDefaults() {
    using WizardSystem::Event;

    Params params;

    params[Param::Magic]->init(0, Event::ResetT1);
    params[Param::Shards]->init(0, Event::ResetT2);
    params[Param::BestMagic]->init(0, Event::ResetT2);
    params[Param::PoisonMagic]->init(0, Event::ResetT1);
    params[Param::PoisonRate]->init(0, Event::ResetT1);

    params[Param::WizardCntUpCost]->init(Number(1, 4));
    params[Param::GlowUpCost]->init(Number(1, 11));
    params[Param::CatalystCost]->init(1);
    params[Param::PoisonWizCost]->init(100);
    params[Param::RobotCost]->init(1000);
    params[Param::T1ResetCost]->init(Number(1, 15));

    params[Param::ResetT1]->init(false);
    params[Param::BoughtWizCntUp]->init(false, Event::ResetT1);
    params[Param::BoughtGlowUp]->init(false, Event::ResetT1);
    params[Param::BoughtPowerWizard]->init(false, Event::ResetT1);
    params[Param::BoughtTimeWizard]->init(false, Event::ResetT1);
    params[Param::GlowActive]->init(false, Event::ResetT2);
    params[Param::BoughtCatalyst]->init(false, Event::ResetT2);
    params[Param::BoughtPoisonWizard]->init(false, Event::ResetT2);
    params[Param::BoughtRobotWizard]->init(false, Event::ResetT2);
}
}  // namespace Constants
}  // namespace Crystal
