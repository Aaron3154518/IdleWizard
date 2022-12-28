#include "CatalystConstants.h"

namespace Catalyst {
namespace Constants {
const unsigned int MSPF = 150, NUM_FRAMES = 5;

const AnimationData& IMG() {
    const static AnimationData IMG{"res/wizards/catalyst.png", 1, 100};
    return IMG;
}

const ParameterSystem::BaseValue REG_FB_CNT = ParameterSystem::Param<CATALYST>(
                                     CatalystParams::FBRegCnt),
                                 POW_FB_CNT = ParameterSystem::Param<CATALYST>(
                                     CatalystParams::FBPowCnt),
                                 POI_FB_CNT = ParameterSystem::Param<CATALYST>(
                                     CatalystParams::FBPoiCnt);

const std::vector<ParameterSystem::BaseValue> FB_CNT_TYPES = {
    ParameterSystem::Param<CATALYST>(CatalystParams::Magic), REG_FB_CNT,
    POI_FB_CNT, POW_FB_CNT};

void setDefaults() {
    using WizardSystem::Event;

    ParameterSystem::Params<CATALYST> params;

    params[CatalystParams::Magic]->init(Number(1, 10));

    params[CatalystParams::FBRegCnt]->init(0, Event::ResetT1);
    params[CatalystParams::FBPowCnt]->init(0, Event::ResetT1);
    params[CatalystParams::FBPoiCnt]->init(0, Event::ResetT1);
    params[CatalystParams::CatRingPoisCnt]->init(0, Event::ResetT1);

    params[CatalystParams::BaseRange]->init(1.25);

    params[CatalystParams::ShardGainUpCost]->init(2);
    params[CatalystParams::MultUpCost]->init(100);

    params[CatalystParams::RangeUpLvl]->init(Event::ResetT2);
    params[CatalystParams::ZapCntUpLvl]->init(Event::ResetT2);
    params[CatalystParams::ZapperUpLvl]->init(Event::ResetT2);
    params[CatalystParams::GainUp1Lvl]->init(Event::ResetT2);
    params[CatalystParams::GainUp2Lvl]->init(Event::ResetT2);
    params[CatalystParams::FBCntLvl]->init(Event::ResetT2);

    ParameterSystem::States states;

    states[State::BoughtCatShardMult]->init(false, Event::ResetT2);
}
}  // namespace Constants
}  // namespace Catalyst
