#include "PowerWizardDefs.h"

namespace PowerWizardDefs {
const AnimationData IMG{"res/wizards/power_wizard_ss.png", 8, 150};

const std::string POWER_UP_IMG = "res/upgrades/power_fireball_upgrade.png";

void setDefaults() {
    using WizardSystem::Event;

    ParameterSystem::Params<POWER_WIZARD> params;

    params[PowerWizardParams::BasePower]->init(5);
    params[PowerWizardParams::BaseSpeed]->init(.25);
    params[PowerWizardParams::BaseFBSpeed]->init(.75);

    params[PowerWizardParams::PowerUpLvl]->init(Event::ResetT1);
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
}  // namespace PowerWizardDefs
