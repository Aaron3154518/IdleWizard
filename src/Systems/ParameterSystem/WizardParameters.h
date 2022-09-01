#ifndef WIZARD_TYPES_H
#define WIZARD_TYPES_H

#include <Utils/Number.h>
#include <Wizards/WizardIds.h>

#include <type_traits>

typedef uint8_t param_t;

// Parameter enums
namespace WizardParams {
enum B : param_t {
    BasePower = 0,
    BaseCrit,
    BaseCritSpread,
    BaseSpeed,
    BaseFBSpeed,

    PowerWizEffect,

    PowerUpLvl,
    CritUpLvl,
    MultiUpLvl,
};

enum N : param_t {
    Power = 0,
    PowerUp,
    PowerUpCost,
    PowerUpMaxLvl,

    Crit,
    CritUp,
    CritSpread,
    CritSpreadUp,
    CritUpCost,

    Speed,
    FBSpeed,
    FBSpeedEffect,

    MultiUp,
    MultiUpCost,
};
}  // namespace WizardParams
namespace CrystalParams {
enum B : param_t {
    Magic = 0,
    Shards,

    CatalystCost,

    BuyPowerWizLvl,
    BuyTimeWizLvl,
    BuyCatalystLvl,
};

enum N : param_t {
    MagicEffect = 0,
    ShardGain,

    T1WizardCost,
};
}  // namespace CrystalParams
namespace CatalystParams {
enum B : param_t {
    Magic = 0,
    Capacity,

    BaseRange,

    RangeUpLvl,
};

enum N : param_t {
    MagicEffect = 0,
    Range,

    RangeUpCost,
    RangeUp,
};
}  // namespace CatalystParams
namespace PowerWizardParams {
enum B : param_t {
    BasePower = 0,
    BaseSpeed,
    BaseFBSpeed,

    PowerUpLvl,
};

enum N : param_t {
    Duration = 0,

    Power,
    PowerUp,
    PowerUpCost,
    FireRingEffect,

    Speed,
    FBSpeed,
    FBSpeedEffect,
};
}  // namespace PowerWizardParams
namespace TimeWizardParams {
enum B : param_t {
    SpeedBaseEffect = 0,
    FreezeBaseEffect,

    FreezeDelay,
    FreezeDuration,

    FreezeUpLvl,
    SpeedUpLvl,
    FBSpeedUpLvl,
};

enum N : param_t {
    SpeedEffect,
    SpeedCost,
    SpeedUp,
    SpeedUpCost,

    FreezeEffect,
    FreezeUp,
    FreezeUpCost,
    ClockSpeed,

    FBSpeedUp,
    FBSpeedCost,
};
}  // namespace TimeWizardParams

// Templates for matching wizards to param enums
template <WizardId, class, class>
struct Pack {};

template <WizardId, class...>
struct WizardTypeMapImpl;

template <WizardId id>
struct WizardTypeMapImpl<id> {
    using BaseType = void;
    using NodeType = void;
};

template <WizardId _id, WizardId id, class B, class N, class... Tail>
struct WizardTypeMapImpl<_id, Pack<id, B, N>, Tail...>
    : public WizardTypeMapImpl<_id, Tail...> {
    static_assert(std::is_same<param_t, std::underlying_type_t<B>>::value,
                  "Parameter enum must be of type param_t");
    static_assert(std::is_same<param_t, std::underlying_type_t<N>>::value,
                  "Parameter enum must be of type param_t");

    using BaseType =
        std::conditional_t<id == _id, B,
                           typename WizardTypeMapImpl<_id, Tail...>::BaseType>;
    using NodeType =
        std::conditional_t<id == _id, N,
                           typename WizardTypeMapImpl<_id, Tail...>::NodeType>;
};

// Wizard-parameter mappings
template <WizardId id>
using WizardTypeMap = WizardTypeMapImpl<
    id, Pack<WIZARD, WizardParams::B, WizardParams::N>,
    Pack<CRYSTAL, CrystalParams::B, CrystalParams::N>,
    Pack<CATALYST, CatalystParams::B, CatalystParams::N>,
    Pack<POWER_WIZARD, PowerWizardParams::B, PowerWizardParams::N>,
    Pack<TIME_WIZARD, TimeWizardParams::B, TimeWizardParams::N>>;

template <WizardId id>
using WizardBaseType = typename WizardTypeMap<id>::BaseType;
template <WizardId id>
using WizardNodeType = typename WizardTypeMap<id>::NodeType;

typedef std::unordered_map<param_t, Number> NumberMap;

#endif
