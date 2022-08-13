#ifndef WIZARD_IDS_H
#define WIZARD_IDS_H

#include <unordered_map>
#include <vector>

enum WizardId : uint8_t {
    CRYSTAL = 0,
    CATALYST,
    WIZARD,

    size
};

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

enum Elevation { WIZARDS = 2, UPGRADES = 1, OVERLAYS = 10 };

#endif