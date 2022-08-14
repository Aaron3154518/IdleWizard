#ifndef WIZARD_IDS_H
#define WIZARD_IDS_H

#include <unordered_map>
#include <vector>

enum WizardId : uint8_t {
    CRYSTAL = 0,
    CATALYST,
    WIZARD,
    POWER_WIZARD,

    size
};

template <class T>
using WizardMap = std::unordered_map<WizardId, T>;

const std::string WIZ_DIR = "res/wizards/";
const WizardMap<std::string> WIZ_IMGS = {
    {CRYSTAL, WIZ_DIR + "crystal.png"},
    {CATALYST, WIZ_DIR + "catalyst.png"},
    {WIZARD, WIZ_DIR + "wizard.png"},
    {POWER_WIZARD, WIZ_DIR + "power_wizard.png"}};

const WizardMap<std::string> WIZ_NAMES = {{CRYSTAL, "Crystal"},
                                          {CATALYST, "Catalyst"},
                                          {WIZARD, "Wizard"},
                                          {POWER_WIZARD, "Power Wizard"}};

enum Elevation { WIZARDS = 2, UPGRADES = 1, OVERLAYS = 10 };

#endif