#include "CatalystConstants.h"

namespace Catalyst {
namespace Constants {
const unsigned int MSPF = 150, NUM_FRAMES = 5;

const AnimationData& IMG() {
    const static AnimationData IMG{"res/wizards/catalyst.png", 1, 100};
    return IMG;
}

const ParameterSystem::BaseValue REG_FB_CNT = Params::get(Param::FBRegCnt),
                                 POW_FB_CNT = Params::get(Param::FBPowCnt),
                                 POI_FB_CNT = Params::get(Param::FBPoiCnt);

const std::vector<ParameterSystem::BaseValue> FB_CNT_TYPES = {
    Params::get(Param::Magic), REG_FB_CNT, POI_FB_CNT, POW_FB_CNT};

void setDefaults() {
    using WizardSystem::Event;

    Params params;

    params[Param::Magic]->init(Number(1, 10));

    params[Param::FBRegCnt]->init(0, Event::ResetT1);
    params[Param::FBPowCnt]->init(0, Event::ResetT1);
    params[Param::FBPoiCnt]->init(0, Event::ResetT1);
    params[Param::CatRingPoisCnt]->init(0, Event::ResetT1);

    params[Param::BaseRange]->init(1.25);

    params[Param::ShardGainUpCost]->init(2);
    params[Param::MultUpCost]->init(100);

    params[Param::RangeUpLvl]->init(Event::ResetT2);
    params[Param::ZapCntUpLvl]->init(Event::ResetT2);
    params[Param::ZapperUpLvl]->init(Event::ResetT2);
    params[Param::GainUp1Lvl]->init(Event::ResetT2);
    params[Param::GainUp2Lvl]->init(Event::ResetT2);
    params[Param::FBCntLvl]->init(Event::ResetT2);

    params[Param::BoughtCatShardMult]->init(false, Event::ResetT2);
}
}  // namespace Constants
}  // namespace Catalyst
