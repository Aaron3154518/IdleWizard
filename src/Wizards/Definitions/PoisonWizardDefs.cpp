#include "PoisonWizardDefs.h"

namespace PoisonWizardDefs {
const AnimationData IMG{"res/wizards/poison_wizard_ss.png", 8, 100},
    GLOB_IMG{"res/projectiles/poison_glob_ss.png", 8, 100},
    BUBBLE1_IMG{"res/projectiles/poison_bubble1_ss.png", 9, 150},
    BUBBLE2_IMG{"res/projectiles/poison_bubble2_ss.png", 6, 150};

void setDefaults() {
    using WizardSystem::Event;

    ParameterSystem::Params<POISON_WIZARD> params;

    params[PoisonWizardParams::BaseSpeed]->init(.125);
    params[PoisonWizardParams::BaseGlobCnt]->init(5);
    params[PoisonWizardParams::PoisonDecay]->init(.95);

    params[PoisonWizardParams::CrysPoisonUpCost]->init(Number(1, 5));
    params[PoisonWizardParams::PoisonFbUpLvl]->init(Event::ResetT2);

    ParameterSystem::States states;
}
}  // namespace PoisonWizardDefs
