#include "PoisonWizardConstants.h"

namespace PoisonWizard {
namespace Constants {
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

    Params params;

    params[Param::BaseSpeed]->init(.125);
    params[Param::BaseGlobCnt]->init(5);
    params[Param::BasePoisonRate]->init(0.1);
    params[Param::PoisonDecay]->init(.95);

    params[Param::CatPoisonUpCost]->init(Number(1, 4));
    params[Param::CatPoisCntUp]->init(5);

    params[Param::PoisonFbUpLvl]->init(Event::ResetT2);
    params[Param::GlobCntUpLvl]->init(Event::ResetT2);
}
}  // namespace Constants
}  // namespace PoisonWizard
