#define SDL_MAIN_HANDLED

#include <Components/Upgrade.h>
#include <GameSystem.h>
#include <Systems/WizardSystem.h>
#include <Wizards/Catalyst.h>
#include <Wizards/Crystal.h>
#include <Wizards/PowerWizard.h>
#include <Wizards/TimeWizard.h>
#include <Wizards/Wizard.h>
#include <Wizards/WizardBase.h>

#include <memory>

int main(int argc, char* argv[]) {
    RenderSystem::Options options;
    options.width = options.height = 750;
    options.title = "Idle Wizard";
    options.defaultTexture = "res/default.png";

    GameSystem::Init(options);

    // Initialize Parameters
    WizardDefs::setDefaults();
    CrystalDefs::setDefaults();
    CatalystDefs::setDefaults();
    PowerWizardDefs::setDefaults();
    TimeWizardDefs::setDefaults();

    {  // Configure starting conditions
        enum Start { None = 0, FirstT1, SecondT1, Fracture };
        Start start = Start::Fracture;
        WizardId t1Wiz = CATALYST, t2Wiz = CATALYST;

        ParameterSystem::States states;
        ParameterSystem::Params<WIZARD> wParams;
        ParameterSystem::Params<POWER_WIZARD> pwParams;
        ParameterSystem::Params<TIME_WIZARD> twParams;
        ParameterSystem::Params<CRYSTAL> cryParams;
        switch (start) {
            case Start::Fracture:
                states[State::BoughtPowerWizard].set(true);
                states[State::BoughtTimeWizard].set(true);

                states[State::BoughtCrysWizCntUp].set(true);
                states[State::BoughtCrysGlowUp].set(true);

                cryParams[CrystalParams::Magic].set(
                    cryParams[CrystalParams::T1ResetCost].get());

                wParams[WizardParams::PowerUpLvl].set(10);
                wParams[WizardParams::CritUpLvl].set(10);
                wParams[WizardParams::MultiUpLvl].set(20);

                pwParams[PowerWizardParams::PowerUpLvl].set(15);
                pwParams[PowerWizardParams::TimeWarpUpLvl].set(6);

                twParams[TimeWizardParams::SpeedUpLvl].set(10);
                twParams[TimeWizardParams::SpeedUpUpLvl].set(8);
                twParams[TimeWizardParams::FBSpeedUpLvl].set(6);
                twParams[TimeWizardParams::FreezeUpLvl].set(8);
                break;
            case Start::SecondT1:
                states[State::BoughtPowerWizard].set(true);
                states[State::BoughtTimeWizard].set(true);
                states[State::BoughtCrysWizCntUp].set(true);
                wParams[WizardParams::PowerUpLvl].set(10);
                switch (t1Wiz) {
                    case POWER_WIZARD:
                        wParams[WizardParams::CritUpLvl].set(10);
                        pwParams[PowerWizardParams::PowerUpLvl].set(15);
                        break;
                    case TIME_WIZARD:
                        wParams[WizardParams::MultiUpLvl].set(20);
                        twParams[TimeWizardParams::SpeedUpLvl].set(10);
                        twParams[TimeWizardParams::FBSpeedUpLvl].set(6);
                        twParams[TimeWizardParams::FreezeUpLvl].set(8);
                        break;
                }
                break;
            case Start::FirstT1:
                wParams[WizardParams::PowerUpLvl].set(5);
                switch (t1Wiz) {
                    case POWER_WIZARD:
                        states[State::BoughtPowerWizard].set(true);
                        break;
                    case TIME_WIZARD:
                        states[State::BoughtTimeWizard].set(true);
                        break;
                }
                break;
        };

        switch (t2Wiz) {
            case CATALYST:
                states[State::BoughtCatalyst].set(true);
                cryParams[CrystalParams::Shards].set(100);
                break;
        }
    }

    // Create Components
    std::unique_ptr<UpgradeScroller> upgradeScroller =
        ComponentFactory<UpgradeScroller>::New();
    std::unique_ptr<Wizard> wizard = ComponentFactory<Wizard>::New();
    std::unique_ptr<Crystal> catalyst = ComponentFactory<Crystal>::New();
    std::unique_ptr<Catalyst> crystal = ComponentFactory<Catalyst>::New();
    std::unique_ptr<PowerWizard> powerWizard =
        ComponentFactory<PowerWizard>::New();
    std::unique_ptr<TimeWizard> timeWizard =
        ComponentFactory<TimeWizard>::New();

    GameSystem::Run();

    GameSystem::Clean();

    return 0;
}