#ifndef WIZARD_IDS_H
#define WIZARD_IDS_H

#include <typeinfo>
#include <unordered_map>
#include <vector>

enum WizardId : uint8_t {
    CRYSTAL = 0,
    CATALYST,
    WIZARD,

    size
};

namespace WizardParams {
enum Params : uint8_t {
    Power = 0,
    PowerUpgrade,
    Speed,

    TYPE
};
}
namespace CrystalParams {
enum Params : uint8_t {
    Magic = 0,
    MagicEffect,

    TYPE
};
}
namespace CatalystParams {
enum Params : uint8_t {
    Magic = 0,
    MagicEffect,
    Capacity,

    TYPE
};
}

template <class T>
using WizardMap = std::unordered_map<WizardId, T>;

const std::string WIZ_DIR = "res/wizards/";
const WizardMap<std::string> WIZ_IMGS = {
    {WizardId::CRYSTAL, WIZ_DIR + "crystal.png"},
    {WizardId::CATALYST, WIZ_DIR + "catalyst.png"},
    {WizardId::WIZARD, WIZ_DIR + "wizard.png"}};

const WizardMap<std::string> WIZ_NAMES = {{WizardId::CRYSTAL, "Crystal"},
                                          {WizardId::CATALYST, "Catalyst"},
                                          {WizardId::WIZARD, "Wizard"}};

const WizardMap<const std::type_info&> WIZ_DATA_TYPES = {
    {WizardId::WIZARD, typeid(WizardParams::Params::TYPE)},
    {WizardId::CRYSTAL, typeid(CrystalParams::Params::TYPE)},
    {WizardId::CATALYST, typeid(CatalystParams::Params::TYPE)}};

enum Elevation { WIZARDS = 2, UPGRADES = 1, OVERLAYS = 10 };

#endif