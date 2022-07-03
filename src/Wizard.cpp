#include "Wizard.h"

#include "IdleWizard.cpp"

const std::string Wizard::IMGS[] = {"res/wizards/crystal.png", "res/wizards/catalyst.png", "res/wizards/wizard.png"};

void Wizard::init() {
    mImg.texture = AssetManager::getTexture(IMGS[imgIdx]);
    mImg.dest = Rect(0, 0, 100, 100);
    mImg.fitToTexture();
    mPos->rect = mImg.dest;

    mRenderSub = ServiceSystem::Get<RenderService>()->Get<RenderObservable>()->subscribe(
        [this](SDL_Renderer* r) {
            TextureBuilder().draw(mImg);
        },
        mPos);
    mMouseSub = ServiceSystem::Get<MouseService>()->Get<MouseObservable>()->subscribe(
        [this](Event::MouseButton b, bool clicked) {
            if (!clicked) {
                return;
            }

            imgIdx = (imgIdx + 1) % 3;
            mImg.texture = AssetManager::getTexture(IMGS[imgIdx]);
            mImg.dest = Rect(0, 0, 100, 100);
            mImg.fitToTexture();
            mPos->rect = mImg.dest;
        },
        mPos);
}