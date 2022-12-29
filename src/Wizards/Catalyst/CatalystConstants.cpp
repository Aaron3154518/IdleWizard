#include "CatalystConstants.h"

namespace Catalyst {
namespace Constants {
const unsigned int MSPF = 150, NUM_FRAMES = 5;

const AnimationData& IMG() {
    const static AnimationData IMG{"res/wizards/catalyst.png", 1, 100};
    return IMG;
}

const ParameterSystem::BaseValue REG_FB_CNT = Catalyst::Params::get(
                                     Catalyst::Param::FBRegCnt),
                                 POW_FB_CNT = Catalyst::Params::get(
                                     Catalyst::Param::FBPowCnt),
                                 POI_FB_CNT = Catalyst::Params::get(
                                     Catalyst::Param::FBPoiCnt);

const std::vector<ParameterSystem::BaseValue> FB_CNT_TYPES = {
    Catalyst::Params::get(Catalyst::Param::Magic), REG_FB_CNT,
    POI_FB_CNT, POW_FB_CNT};

void setDefaults() {
    using WizardSystem::Event;

    Catalyst::Params params;

    params[Catalyst::Param::Magic]->init(Number(1, 10));

    params[Catalyst::Param::FBRegCnt]->init(0, Event::ResetT1);
    params[Catalyst::Param::FBPowCnt]->init(0, Event::ResetT1);
    params[Catalyst::Param::FBPoiCnt]->init(0, Event::ResetT1);
    params[Catalyst::Param::CatRingPoisCnt]->init(0, Event::ResetT1);

    params[Catalyst::Param::BaseRange]->init(1.25);

    params[Catalyst::Param::ShardGainUpCost]->init(2);
    params[Catalyst::Param::MultUpCost]->init(100);

    params[Catalyst::Param::RangeUpLvl]->init(Event::ResetT2);
    params[Catalyst::Param::ZapCntUpLvl]->init(Event::ResetT2);
    params[Catalyst::Param::ZapperUpLvl]->init(Event::ResetT2);
    params[Catalyst::Param::GainUp1Lvl]->init(Event::ResetT2);
    params[Catalyst::Param::GainUp2Lvl]->init(Event::ResetT2);
    params[Catalyst::Param::FBCntLvl]->init(Event::ResetT2);


    states[State::BoughtCatShardMult]->init(false, Event::ResetT2);
}
}  // namespace Constants
}  // namespace Catalyst
