#ifndef WIZARD_IDS_H
#define WIZARD_IDS_H

enum WizardId : uint8_t {
    CRYSTAL = 0,
    CATALYST,
    WIZARD,

    size
};

const std::string WIZ_DIR = "res/wizards/";
const std::string WIZ_IMGS[WizardId::size + 1] = {WIZ_DIR + "crystal.png",
                                                  WIZ_DIR + "catalyst.png",
                                                  WIZ_DIR + "wizard.png", ""};

enum WizardParams : int { CrystalMagic = 0, CatalystMagic, WizardPowerUpgrade };

enum Elevation { WIZARDS = 1, UPGRADES = 2 };

#endif