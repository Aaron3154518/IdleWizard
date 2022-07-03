#define SDL_MAIN_HANDLED

#include <GameSystem.h>

#include <memory>

#include "Wizard.h"

int main(int argc, char* argv[]) {
    RenderSystem::Options options;
    options.title = "Idle Wizard";
    options.maximize = true;
    options.defaultTexture = "res/default.png";

    GameSystem::Init(options);

    std::unique_ptr<Wizard> wizard = ComponentFactory<Wizard>::New();

    GameSystem::Run();

    GameSystem::Clean();

    return 0;
}