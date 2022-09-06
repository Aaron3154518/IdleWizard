#include "RobotWizardDefs.h"

namespace RobotWizardDefs {
const std::vector<WizardId> TARGETS{WIZARD, TIME_WIZARD, POWER_WIZARD, CRYSTAL};

const AnimationData IMG{"res/wizards/robot_ss.png", 6, 100};

void setDefaults() {
    using WizardSystem::Event;

    ParameterSystem::Params<CRYSTAL> params;

    ParameterSystem::States states;
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
}  // namespace RobotWizardDefs
