#ifndef WIZARD_TYPES_H
#define WIZARD_TYPES_H

#include <Utils/Number.h>
#include <Wizards/WizardIds.h>

#include <type_traits>

typedef uint8_t param_t;

// Parameter enums
namespace WizardParams {
enum _ : param_t {
    BasePower = 0,
    Power,
    PowerWizEffect,
    PowerUp,
    PowerUpCost,

    BaseSpeed,
    Speed,
    SpeedUpCost,

    MultiUp,
    MultiUpCost,
};
}
namespace CrystalParams {
enum _ : param_t {
    Magic = 0,
    MagicEffect,

    T1WizardCost
};
}
namespace CatalystParams {
enum _ : param_t {
    Magic = 0,
    MagicEffect,
    Capacity,
};
}
namespace PowerWizardParams {
enum _ : param_t {
    BasePower = 0,
    Power,
    PowerUp,
    PowerUpCost,
    FireRingEffect,

    BaseSpeed,
    Speed,

    Duration,
};
}
namespace TimeWizardParams {
enum _ : param_t {
    SpeedBaseEffect = 0,
    SpeedEffect,
    SpeedCost,
    SpeedUp,
    SpeedUpCost,

    FreezeDelay,
    FreezeDuration,
    FreezeBaseEffect,
    FreezeEffect,
    FreezeUp,
    FreezeUpCost,
};
}

// Templates for matching wizards to param enums
template <WizardId id, class T>
struct Pair {};

template <WizardId, class...>
struct WizardTypeMapImpl;

template <WizardId id>
struct WizardTypeMapImpl<id> {
    using type = void;
};

template <WizardId _id, WizardId id, class T, class... Tail>
struct WizardTypeMapImpl<_id, Pair<id, T>, Tail...>
    : public WizardTypeMapImpl<_id, Tail...> {
    static_assert(std::is_same<param_t, std::underlying_type_t<T>>::value,
                  "Parameter enum must of type param_t (aka size_t)");

    using type =
        std::conditional_t<id == _id, T,
                           typename WizardTypeMapImpl<_id, Tail...>::type>;
};

// Wizard-parameter mappings
template <WizardId id>
using WizardTypeMap = WizardTypeMapImpl<
    id, Pair<WIZARD, WizardParams::_>, Pair<CRYSTAL, CrystalParams::_>,
    Pair<CATALYST, CatalystParams::_>, Pair<POWER_WIZARD, PowerWizardParams::_>,
    Pair<TIME_WIZARD, TimeWizardParams::_>>;

template <WizardId id>
using WizardType = typename WizardTypeMap<id>::type;

typedef std::unordered_map<param_t, Number> NumberMap;

#endif
