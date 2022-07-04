#define SDL_MAIN_HANDLED

#include <GameSystem.h>

#include <memory>

#include "Wizard.h"
#include "WizardIds.h"

int main(int argc, char* argv[]) {
    RenderSystem::Options options;
    options.width = options.height = 750;
    options.title = "Idle Wizard";
    options.defaultTexture = "res/default.png";

    GameSystem::Init(options);

    std::unique_ptr<Wizard> wizard = ComponentFactory<Wizard>::New(WizardId::WIZARD);
    std::unique_ptr<Wizard> catalyst = ComponentFactory<Wizard>::New(WizardId::CATALYST);
    std::unique_ptr<Wizard> crystal = ComponentFactory<Wizard>::New(WizardId::CRYSTAL);

    GameSystem::Run();

    GameSystem::Clean();

    return 0;
}