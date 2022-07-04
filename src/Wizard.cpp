#include "Wizard.h"

// Wizard;
const std::string Wizard::IMGS[] = {"res/wizards/crystal.png", "res/wizards/catalyst.png", "res/wizards/wizard.png"};

void Wizard::init() {
    Rect borderRect(0, 0, 500, 500);

    mBorder.set(borderRect, 2, true);

    mPos->rect.setCenter(borderRect.cX(), borderRect.cY());
    setImage(IMGS[imgIdx]);

    mPos->onDrag = [this, borderRect](int x, int y, double dx, double dy) {
        mPos->rect.setCenter(x, y);
        mPos->rect.x = std::max(std::min(mPos->rect.x, borderRect.x2() - mPos->rect.w), 0);
        mPos->rect.y = std::max(std::min(mPos->rect.y, borderRect.y2() - mPos->rect.h), 0);
        mImg.dest = mPos->rect;
    };
    mPos->onDragStart = []() {};
    mPos->onDragEnd = []() {};

    mRenderSub = ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
        [this](SDL_Renderer* r) {
            if (mPos->dragging) {
                RectData rd;
                rd.color = GRAY;
                rd.set(mPos->rect, 5);
                TextureBuilder().draw(rd);
            }

            TextureBuilder().draw(mImg);

            TextureBuilder().draw(mBorder);
        },
        mPos);
    mMouseSub = ServiceSystem::Get<MouseService, MouseObservable>()->subscribe(
        [this](Event::MouseButton b, bool clicked) {
            if (!clicked) {
                return;
            }

            imgIdx = (imgIdx + 1) % 3;
            setImage(IMGS[imgIdx]);
        },
        mPos);
    mDragSub = ServiceSystem::Get<DragService, DragObservable>()->subscribe(mPos);
}

void Wizard::setImage(const std::string& img) {
    const static Rect IMG_RECT(0, 0, 100, 100);
    mImg.texture = AssetManager::getTexture(img);
    mImg.dest = IMG_RECT;
    mImg.dest.setCenter(mPos->rect.cX(), mPos->rect.cY());
    mImg.fitToTexture();
    mPos->rect = mImg.dest;
}