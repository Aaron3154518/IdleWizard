#define SDL_MAIN_HANDLED

#include <Components/Upgrade.h>
#include <GameSystem.h>
#include <Systems/WizardSystem.h>
#include <Wizards/Catalyst.h>
#include <Wizards/Crystal.h>
#include <Wizards/PoisonWizard.h>
#include <Wizards/PowerWizard.h>
#include <Wizards/RobotWizard.h>
#include <Wizards/TimeWizard.h>
#include <Wizards/Wizard.h>
#include <Wizards/WizardBase.h>

#include <memory>

int main(int argc, char* argv[]) {
    RenderSystem::Options options;
    options.width = options.height = 1000;
    options.title = "Idle Wizard";
    options.defaultTexture = "res/default.png";

    GameSystem::Init(options);

    // Initialize Parameters
    WizardDefs::setDefaults();
    CrystalDefs::setDefaults();
    CatalystDefs::setDefaults();
    PowerWizardDefs::setDefaults();
    TimeWizardDefs::setDefaults();
    PowerWizardDefs::setDefaults();
    RobotWizardDefs::setDefaults();

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
    std::unique_ptr<PoisonWizard> poisonWizard =
        ComponentFactory<PoisonWizard>::New();
    std::unique_ptr<RobotWizard> robotWizard =
        ComponentFactory<RobotWizard>::New();

    {  // Configure starting conditions
        enum Start { None = 0, FirstT1, SecondT1, Fracture };
        Start start = Start::SecondT1;
        WizardId t1Wiz = POWER_WIZARD, t2Wiz = CATALYST;

        ParameterSystem::States states;
        ParameterSystem::Params<WIZARD> wParams;
        ParameterSystem::Params<POWER_WIZARD> pwParams;
        ParameterSystem::Params<TIME_WIZARD> twParams;
        ParameterSystem::Params<CRYSTAL> cryParams;
        // Buy upgrades
        switch (start) {
            case Start::Fracture:
                states[State::BoughtPowerWizard].set(true);
                states[State::BoughtTimeWizard].set(true);

                cryParams[CrystalParams::Magic].set(
                    cryParams[CrystalParams::T1ResetCost].get());

                for (WizardId id :
                     {CRYSTAL, WIZARD, POWER_WIZARD, TIME_WIZARD}) {
                    GetWizardUpgrades(id)->buyAllFree(
                        UpgradeDefaults::CRYSTAL_MAGIC);
                }
                break;
            case Start::SecondT1:
                switch (t1Wiz) {
                    case POWER_WIZARD:
                        states[State::BoughtTimeWizard].set(true);
                        GetWizardUpgrades(TIME_WIZARD)
                            ->buyAllFree(UpgradeDefaults::CRYSTAL_MAGIC);
                        break;
                    case TIME_WIZARD:
                        states[State::BoughtPowerWizard].set(true);
                        GetWizardUpgrades(POWER_WIZARD)
                            ->buyAllFree(UpgradeDefaults::CRYSTAL_MAGIC);
                        break;
                }
                GetWizardUpgrades(CRYSTAL)->buyAllFree(
                    UpgradeDefaults::CRYSTAL_MAGIC);
            case Start::FirstT1:
                GetWizardUpgrades(WIZARD)->buyAllFree(
                    UpgradeDefaults::CRYSTAL_MAGIC);
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

    GameSystem::Run();

    GameSystem::Clean();

    return 0;
}