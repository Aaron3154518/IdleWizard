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
    mUpdateSub = TimeSystem::GetUpdateObservable()->subscribe(
        std::bind(&FireRing::onUpdate, this, std::placeholders::_1));
    mRingSub =
        ServiceSystem::Get<FireRingService, HitObservable>()
            ->subscribeToFireballs(FireballObservable(mCircle.c, mCircle.r2));
}

void FireRing::onRender(SDL_Renderer* r) { TextureBuilder().draw(mCircle); }

void FireRing::onUpdate(Time dt) {
    mCircle.set(mCircle.c, mCircle.r1 + GROWTH * dt.s(), WIDTH);
    mRingSub->get<0>().next(mCircle.c, mCircle.r2, mEffect);
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

// FireRing::FireballObservable
FireRing::FireballObservable::FireballObservable(SDL_Point c, int r)
    : mC(c), mR(r) {}

void FireRing::FireballObservable::next(SDL_Point c, int r,
                                        const Number& effect) {
    for (auto it = begin(), endIt = end(); it != endIt; ++it) {
        auto sub = *it;
        UIComponentPtr pos = sub->get<DATA>();
        float dx = c.x - pos->rect.cX(), dy = c.y - pos->rect.cY();
        float mag = sqrt(dx * dx + dy * dy);
        if (mag < r) {
            sub->get<FUNC>()(effect);
            it = erase(it);
            if (it == endIt) {
                break;
            }
        }
    }
    mC = c, mR = r;
}

void FireRing::FireballObservable::subscribe(SubscriptionPtr sub) {
    UIComponentPtr pos = sub->get<DATA>();
    float dx = mC.x - pos->rect.cX(), dy = mC.y - pos->rect.cY();
    float mag = sqrt(dx * dx + dy * dy);
    if (mag >= mR) {
        FireballObservableBase::subscribe(sub);
    }
}
