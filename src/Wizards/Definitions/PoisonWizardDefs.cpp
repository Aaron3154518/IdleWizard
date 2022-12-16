#include "PoisonWizardDefs.h"

namespace PoisonWizardDefs {
const AnimationData& IMG() {
    const static AnimationData IMG{"res/wizards/poison_wizard_ss.png", 8, 100};
    return IMG;
}
const AnimationData& GLOB_IMG() {
    const static AnimationData GLOB_IMG{"res/projectiles/poison_glob_ss.png", 8,
                                        100};
    return GLOB_IMG;
}
const AnimationData& BUBBLE1_IMG() {
    const static AnimationData BUBBLE1_IMG{
        "res/projectiles/poison_bubble1_ss.png", 9, 150};
    return BUBBLE1_IMG;
}
const AnimationData& BUBBLE2_IMG() {
    const static AnimationData BUBBLE2_IMG{
        "res/projectiles/poison_bubble2_ss.png", 6, 150};
    return BUBBLE2_IMG;
}

void setDefaults() {
    using WizardSystem::Event;

    ParameterSystem::Params<POISON_WIZARD> params;

    params[PoisonWizardParams::BaseSpeed]->init(.125);
    params[PoisonWizardParams::BaseGlobCnt]->init(5);
    params[PoisonWizardParams::BasePoisonRate]->init(0.1);
    params[PoisonWizardParams::PoisonDecay]->init(.95);

    params[PoisonWizardParams::CatPoisonUpCost]->init(Number(1, 4));
    params[PoisonWizardParams::CatPoisCntUp]->init(5);

    params[PoisonWizardParams::PoisonFbUpLvl]->init(Event::ResetT2);
    params[PoisonWizardParams::GlobCntUpLvl]->init(Event::ResetT2);

    ParameterSystem::States states;
}
}  // namespace PoisonWizardDefs
