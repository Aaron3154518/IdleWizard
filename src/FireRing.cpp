#include "FireRing.h"

// FireRing
const int FireRing::GROWTH = 300;
const int FireRing::WIDTH = 30;
const SDL_Color FireRing::COLOR = {21, 101, 192, 255};

FireRing::FireRing(SDL_Point c, const Number& effect) : mEffect(effect) {
    mCircle.set(c, 0, WIDTH).color = COLOR;
}

void FireRing::init() {
    UIComponentPtr pos =
        std::make_shared<UIComponent>(Rect(), Elevation::EFFECTS);
    pos->mouse = false;
    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&FireRing::onRender, this, std::placeholders::_1), pos);
    mUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            std::bind(&FireRing::onUpdate, this, std::placeholders::_1));
}

void FireRing::onRender(SDL_Renderer* r) { TextureBuilder().draw(mCircle); }

void FireRing::onUpdate(Time dt) {
    mCircle.set(mCircle.c, mCircle.r1 + GROWTH * dt.s(), WIDTH);
    ServiceSystem::Get<FireRingService, FireRingObservable>()->next(
        {(float)mCircle.c.x, (float)mCircle.c.y}, mCircle.r2, mEffect);
    SDL_Point dim = RenderSystem::getWindowSize();
    float dx = fmax(mCircle.c.x, dim.x - mCircle.c.x),
          dy = fmax(mCircle.c.y, dim.y - mCircle.c.y);
    float mag = sqrt(dx * dx + dy * dy);
    if (mag < mCircle.r1) {
        mDead = true;
        mUpdateSub.reset();
        mRenderSub.reset();
    }
}

bool FireRing::dead() const { return mDead; }

// FireRingObservable
void FireRingObservable::next(SDL_FPoint c, int r, const Number& effect) {
    for (auto sub : *this) {
        UIComponentPtr pos = sub->get<DATA>();
        float dx = c.x - pos->rect.cX(), dy = c.y - pos->rect.cY();
        float mag = sqrt(dx * dx + dy * dy);
        if (mag < r) {
            sub->get<FUNC>()(effect);
        }
    }
}
