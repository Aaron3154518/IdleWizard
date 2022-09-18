#include "CatalystDefs.h"

namespace CatalystDefs {
const unsigned int MSPF = 150, NUM_FRAMES = 5;

const AnimationData IMG{"res/wizards/catalyst.png", 1, 100};

void setDefaults() {
    using WizardSystem::Event;

    ParameterSystem::Params<CATALYST> params;

    params[CatalystParams::Magic]->init(Number(1, 10));
    params[CatalystParams::Capacity]->init(Number(1, 10));

    params[CatalystParams::FBRegCnt]->init(0, Event::ResetT1);
    params[CatalystParams::FBBuffCnt]->init(0, Event::ResetT1);
    params[CatalystParams::FBPoisCnt]->init(0, Event::ResetT1);

    params[CatalystParams::BaseRange]->init(1.25);

    params[CatalystParams::ShardGainUpCost]->init(2);

    params[CatalystParams::RangeUpLvl]->init(Event::ResetT2);
    params[CatalystParams::ZapCntUpLvl]->init(Event::ResetT2);

    ParameterSystem::States states;

    states[State::BoughtCatShardMult]->init(false, Event::ResetT2);
}
}  // namespace CatalystDefs
