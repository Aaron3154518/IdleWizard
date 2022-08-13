#define SDL_MAIN_HANDLED

#include <GameSystem.h>

#include <memory>

#include "Upgrade.h"
#include "Wizard.h"
#include "WizardData.h"
#include "WizardIds.h"

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

    GameSystem::Run();

    GameSystem::Clean();

    return 0;
}