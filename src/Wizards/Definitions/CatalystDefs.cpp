#include "CatalystDefs.h"

namespace CatalystDefs {
const unsigned int MSPF = 150, NUM_FRAMES = 5;

const std::string IMG = "res/wizards/catalyst.png";

void setDefaults() {
    using WizardSystem::Event;

    ParameterSystem::Params<CATALYST> params;

    params[CatalystParams::Magic]->init(Number(1, 10));
    params[CatalystParams::Capacity]->init(Number(1, 10));
    params[CatalystParams::BaseRange]->init(1.25);

    params[CatalystParams::ShardGainUpCost]->init(2);

    params[CatalystParams::RangeUpLvl]->init(Event::ResetT2);
    params[CatalystParams::ZapCntUpLvl]->init(Event::ResetT2);

    ParameterSystem::States states;

    states[State::BoughtCatShardMult]->init(false, Event::ResetT2);
}
}  // namespace CatalystDefs
