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