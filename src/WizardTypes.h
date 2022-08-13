#ifndef WIZARD_TYPES_H
#define WIZARD_TYPES_H

#include <type_traits>

#include "WizardIds.h"

namespace WizardParams {
enum _ : uint8_t {
    Power = 0,
    PowerUpgrade,
    Speed,
};
}
namespace CrystalParams {
enum _ : uint8_t {
    Magic = 0,
    MagicEffect,
};
}
namespace CatalystParams {
enum _ : uint8_t {
    Magic = 0,
    MagicEffect,
    Capacity,
};
}

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
    using type =
        std::conditional_t<id == _id, T,
                           typename WizardTypeMapImpl<_id, Tail...>::type>;
};

template <WizardId id>
using WizardTypeMap =
    WizardTypeMapImpl<id, Pair<WizardId::WIZARD, WizardParams::_>,
                      Pair<WizardId::CRYSTAL, CrystalParams::_>,
                      Pair<WizardId::CATALYST, CatalystParams::_>>;

template <WizardId id>
using WizardType = typename WizardTypeMap<id>::type;

#endif
