#define SDL_MAIN_HANDLED

#include <Components/Upgrade.h>
#include <Components/WizardBase.h>
#include <GameSystem.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Wizards/Catalyst/Catalyst.h>
#include <Wizards/Crystal/Crystal.h>
#include <Wizards/PoisonWizard/PoisonWizard.h>
#include <Wizards/PowerWizard/PowerWizard.h>
#include <Wizards/RobotWizard/RobotWizard.h>
#include <Wizards/TimeWizard/TimeWizard.h>
#include <Wizards/Wizard/Wizard.h>

#include <memory>

int main(int argc, char* argv[]) {
    RenderSystem::Options options;
    options.width = options.height = 1000;
    options.title = "Idle Wizard";
    options.defaultTexture = "res/default.png";

    GameSystem::Init(options);

    // Initialize Parameters
    Wizard::Constants::setDefaults();
    Crystal::Constants::setDefaults();
    Catalyst::Constants::setDefaults();
    PowerWizard::Constants::setDefaults();
    TimeWizard::Constants::setDefaults();
    PoisonWizard::Constants::setDefaults();
    RobotWizard::Constants::setDefaults();

    // Create Components
    std::unique_ptr<UpgradeDisplay> upgradeDisplay =
        ComponentFactory<UpgradeDisplay>::New();
    std::unique_ptr<Wizard::Wizard> wizard =
        ComponentFactory<Wizard::Wizard>::New();
    std::unique_ptr<Crystal::Crystal> catalyst =
        ComponentFactory<Crystal::Crystal>::New();
    std::unique_ptr<Catalyst::Catalyst> crystal =
        ComponentFactory<Catalyst::Catalyst>::New();
    std::unique_ptr<PowerWizard::PowerWizard> powerWizard =
        ComponentFactory<PowerWizard::PowerWizard>::New();
    std::unique_ptr<TimeWizard::TimeWizard> timeWizard =
        ComponentFactory<TimeWizard::TimeWizard>::New();
    std::unique_ptr<PoisonWizard::PoisonWizard> poisonWizard =
        ComponentFactory<PoisonWizard::PoisonWizard>::New();
    std::unique_ptr<RobotWizard::RobotWizard> robotWizard =
        ComponentFactory<RobotWizard::RobotWizard>::New();

    {  // Configure starting conditions
        enum Start { None = 0, FirstT1, SecondT1, Fracture, T2 };
        Start start = Start::T2;
        WizardId t1Wiz = POWER_WIZARD, t2Wiz = CATALYST;

            Wizard::Params wParams;
        PowerWizard::Params pwParams;
        TimeWizard::Params twParams;
        Crystal::Params cryParams;
        // Buy upgrades
        switch (start) {
            case Start::T2:
                params[Param::ResetT1].set(true);

                // GetWizardUpgrades(CRYSTAL)->maxAll(
                // UpgradeDefaults::CRYSTAL_SHARDS);
                // GetWizardUpgrades(CRYSTAL)->maxAll(
                // UpgradeDefaults::CRYSTAL_SHARDS);
                params[Crystal::Param::BoughtRobotWizard].set(true);
                cryParams[Crystal::Param::Shards].set(Number(1, 10));
            case Start::Fracture:
                params[Crystal::Param::BoughtPowerWizard].set(true);
                params[Crystal::Param::BoughtTimeWizard].set(true);

                cryParams[Crystal::Param::Magic].set(
                    cryParams[Crystal::Param::T1ResetCost].get());

                for (WizardId id : {
                         CRYSTAL,
                     }) {  // WIZARD, POWER_WIZARD, TIME_WIZARD}) {
                    GetWizardUpgrades(id)->maxAll(
                        UpgradeDefaults::CRYSTAL_MAGIC);
                }
                break;
            case Start::SecondT1:
                switch (t1Wiz) {
                    case POWER_WIZARD:
                        params[Crystal::Param::BoughtTimeWizard].set(true);
                        GetWizardUpgrades(TIME_WIZARD)
                            ->maxAll(UpgradeDefaults::CRYSTAL_MAGIC);
                        break;
                    case TIME_WIZARD:
                        params[Crystal::Param::BoughtPowerWizard].set(true);
                        GetWizardUpgrades(POWER_WIZARD)
                            ->maxAll(UpgradeDefaults::CRYSTAL_MAGIC);
                        break;
                }
                GetWizardUpgrades(CRYSTAL)->maxAll(
                    UpgradeDefaults::CRYSTAL_MAGIC);
            case Start::FirstT1:
                // GetWizardUpgrades(WIZARD)->maxAll(
                // UpgradeDefaults::CRYSTAL_MAGIC);
                switch (t1Wiz) {
                    case POWER_WIZARD:
                        params[Crystal::Param::BoughtPowerWizard].set(true);
                        break;
                    case TIME_WIZARD:
                        params[Crystal::Param::BoughtTimeWizard].set(true);
                        break;
                }
                break;
        };
    }

    GameSystem::Run();

    GameSystem::Clean();

    return 0;
}