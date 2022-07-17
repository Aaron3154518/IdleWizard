#include "Wizard.h"

// Wizard;
const Rect Wizard::BORDER_RECT(0, 0, 500, 500), Wizard::IMG_RECT(0, 0, 100,
                                                                 100);
const std::string Wizard::IMGS[] = {"res/wizards/crystal.png",
                                    "res/wizards/catalyst.png",
                                    "res/wizards/wizard.png"};

Wizard::Wizard(WizardId id)
    : mId(id),
      mComp(std::make_shared<DragComponent>(Rect(), 1, 250)),
      gen(rd()),
      dist(1, WizardId::size - 1) {}

void Wizard::init() {
    mBorder.set(BORDER_RECT, 2, true);

    setPos(BORDER_RECT.cX(), BORDER_RECT.cY());
    setImage(IMGS[mId]);

    mComp->onDrag = [this](int x, int y, float dx, float dy) { setPos(x, y); };
    mComp->onDragStart = []() {};
    mComp->onDragEnd = []() {};

    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&Wizard::onRender, this, std::placeholders::_1), mComp);
    mMouseSub = ServiceSystem::Get<MouseService, MouseObservable>()->subscribe(
        std::bind(&Wizard::onClick, this, std::placeholders::_1,
                  std::placeholders::_2),
        mComp);
    mDragSub =
        ServiceSystem::Get<DragService, DragObservable>()->subscribe(mComp);
}

void Wizard::onRender(SDL_Renderer* r) {
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
}
void Wizard::onClick(Event::MouseButton b, bool clicked) {
    if (clicked) {
        shootFireball(
            static_cast<WizardId>((mId + dist(gen)) % WizardId::size));
    }
}

void Wizard::setPos(float x, float y) {
    mComp->rect.setPos(x, y, Rect::Align::CENTER);
    mComp->rect.fitWithin(BORDER_RECT);
    mImg.dest = mComp->rect;
    ServiceSystem::Get<FireballService, FireballObservable>()->next(
        mId, {mComp->rect.cX(), mComp->rect.cY()});
}

void Wizard::setImage(const std::string& img) {
    mImg.texture = AssetManager::getTexture(img);
    mImg.dest = IMG_RECT;
    mImg.dest.setPos(mComp->rect.cX(), mComp->rect.cY(), Rect::Align::CENTER);
    mImg.fitToTexture();
    mComp->rect = mImg.dest;
}

void Wizard::shootFireball(WizardId target) {
    mFireballs.push_back(std::move(ComponentFactory<Fireball>::New(
        mComp->rect.cX(), mComp->rect.cY(), target)));
}