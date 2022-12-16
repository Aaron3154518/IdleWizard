#include "WizardDefs.h"

constexpr int FB_FRAMES = 6, FB_MSPF = 75;

namespace WizardDefs {
const AnimationData& IMG() {
    const static AnimationData IMG{"res/wizards/wizard_ss.png", 5, 150};
    return IMG;
}
const AnimationData& POWER_BKGRND() {
    const static AnimationData POWER_BKGRND{
        "res/wizards/power_effect_bkgrnd_ss.png", 6, 100};
    return POWER_BKGRND;
}
const AnimationData& FB_IMG() {
    const static AnimationData FB_IMG{"res/projectiles/fireball_ss.png",
                                      FB_FRAMES, FB_MSPF};
    return FB_IMG;
}
const AnimationData& FB_BUFFED_IMG() {
    const static AnimationData FB_BUFFED_IMG{
        "res/projectiles/fireball_buffed_ss.png", FB_FRAMES, FB_MSPF};
    return FB_BUFFED_IMG;
}
const AnimationData& FB_POISON_IMG() {
    const static AnimationData FB_POISON_IMG{
        "res/projectiles/fireball_poison_ss.png", FB_FRAMES, FB_MSPF};
    return FB_POISON_IMG;
}
const AnimationData& FB_INNER_IMG() {
    const static AnimationData FB_INNER_IMG{"res/projectiles/fb_inner_ss.png",
                                            FB_FRAMES, FB_MSPF};
    return FB_INNER_IMG;
}
const AnimationData& FB_OUTER_IMG() {
    const static AnimationData FB_OUTER_IMG{"res/projectiles/fb_outer_ss.png",
                                            FB_FRAMES, FB_MSPF};
    return FB_OUTER_IMG;
}
const AnimationData& FB_OUTER_BUFFED_IMG() {
    const static AnimationData FB_OUTER_BUFFED_IMG{
        "res/projectiles/fb_outer_buffed_ss.png", FB_FRAMES, FB_MSPF};
    return FB_OUTER_BUFFED_IMG;
}
const AnimationData& FB_INNER_POISON_IMG() {
    const static AnimationData FB_INNER_POISON_IMG{
        "res/projectiles/fb_inner_poison_ss.png", FB_FRAMES, FB_MSPF};
    return FB_INNER_POISON_IMG;
}

const std::string POWER_UP_IMG = "res/upgrades/fireball_upgrade.png";
const std::string MULTI_UP_IMG = "res/upgrades/multi_upgrade.png";
const std::string CRIT_UP_IMG = "res/upgrades/crit_upgrade.png";

const std::vector<WizardId> TARGETS = {CRYSTAL, CATALYST};

void setDefaults() {
    using WizardSystem::Event;

    ParameterSystem::Params<WIZARD> params;

    params[WizardParams::BaseCritSpread]->init(0);
    params[WizardParams::BasePower]->init(1);
    params[WizardParams::BaseSpeed]->init(.5);
    params[WizardParams::BaseFBSpeed]->init(1);
    params[WizardParams::BaseCrit]->init(1);
    params[WizardParams::PowerWizEffect]->init(1, Event::ResetT1);

    params[WizardParams::PowerUpLvl]->init(Event::ResetT1);
    params[WizardParams::MultiUpLvl]->init(Event::ResetT1);
    params[WizardParams::CritUpLvl]->init(Event::ResetT1);
    params[WizardParams::RoboCritUpLvl]->init(Event::ResetT1);

    ParameterSystem::States states;
    states[State::WizBoosted]->init(false, Event::ResetT1);
}
}  // namespace WizardDefs
