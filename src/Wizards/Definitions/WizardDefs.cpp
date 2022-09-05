#include "WizardDefs.h"

namespace WizardDefs {
const AnimationData IMG{"res/wizards/wizard_ss.png", 5, 150},
    POWER_BKGRND{"res/wizards/power_effect_bkgrnd_ss.png", 6, 100};

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

    params[WizardParams::CritUpLvl]->init(Event::ResetT1);
    params[WizardParams::MultiUpLvl]->init(Event::ResetT1);
    params[WizardParams::PowerUpLvl]->init(Event::ResetT1);

    ParameterSystem::States states;
    states[State::WizBoosted]->init(false, Event::ResetT1);
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
}  // namespace WizardDefs
