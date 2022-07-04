#include "Wizard.h"

// Wizard;
const Rect Wizard::BORDER_RECT(0, 0, 500, 500), Wizard::IMG_RECT(0, 0, 100, 100);
const std::string Wizard::IMGS[] = {"res/wizards/crystal.png", "res/wizards/catalyst.png", "res/wizards/wizard.png"};

Wizard::Wizard(WizardId id)
    : mId(id), mComp(std::make_shared<DragComponent>(Rect(), 1, 250)), gen(rd()), dist(1, WizardId::size - 1) {}

void Wizard::init() {
    mBorder.set(BORDER_RECT, 2, true);

    setPos(BORDER_RECT.cX(), BORDER_RECT.cY());
    setImage(IMGS[mId]);

    mComp->onDrag = [this](int x, int y, double dx, double dy) {
        setPos(x, y);
    };
    mComp->onDragStart = []() {};
    mComp->onDragEnd = []() {};

    mRenderSub = ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
        [this](SDL_Renderer* r) {
            if (mComp->dragging) {
                RectData rd;
                rd.color = GRAY;
                rd.set(mComp->rect, 5);
                TextureBuilder().draw(rd);
            }

            TextureBuilder().draw(mImg);

            TextureBuilder().draw(mBorder);

            for (auto it = mFireballs.begin(); it != mFireballs.end(); ++it) {
                if (!(*it)->dead()) {
                    it = mFireballs.erase(it);
                    if (it == mFireballs.end()) {
                        break;
                    }
                }
            }
        },
        mComp);
    mMouseSub = ServiceSystem::Get<MouseService, MouseObservable>()->subscribe(
        [this](Event::MouseButton b, bool clicked) {
            if (!clicked) {
                return;
            }

            WizardId target = static_cast<WizardId>((mId + dist(gen)) % WizardId::size);
            mFireballs.push_back(std::move(ComponentFactory<Fireball>::New(mComp->rect.center(), target)));
        },
        mComp);
    mDragSub = ServiceSystem::Get<DragService, DragObservable>()->subscribe(mComp);
}

void Wizard::setPos(float x, float y) {
    mPos.x = x;
    mPos.y = y;
    mComp->rect.setCenter(x, y);
    mComp->rect.x = std::max(std::min(mComp->rect.x, BORDER_RECT.x2() - mComp->rect.w), 0);
    mComp->rect.y = std::max(std::min(mComp->rect.y, BORDER_RECT.y2() - mComp->rect.h), 0);
    mImg.dest = mComp->rect;
    ServiceSystem::Get<FireballService, FireballObservable>()->next(mId, mComp->rect.center());
}

void Wizard::setImage(const std::string& img) {
    mImg.texture = AssetManager::getTexture(img);
    mImg.dest = IMG_RECT;
    mImg.dest.setCenter(mComp->rect.cX(), mComp->rect.cY());
    mImg.fitToTexture();
    mComp->rect = mImg.dest;
}

void Wizard::shootFireball(WizardId target) {}