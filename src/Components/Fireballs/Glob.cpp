#include "Glob.h"

// HitObservable
Glob::HitObservable::Side Glob::HitObservable::next(const Rect& rect) {
    for (auto sub : *this) {
        auto& pos = sub->get<DATA>();
        if (pos->visible) {
            SDL_Rect r;
            if (SDL_IntersectRect(pos->rect, rect, &r)) {
                sub->get<FUNC>()();
                return Side::LEFT;
            }
        }
    }

    return Side::NONE;
}

std::shared_ptr<Glob::HitObservable> Glob::GetHitObservable() {
    return ServiceSystem::Get<GlobService, Glob::HitObservable>();
}

// Glob
const Rect Glob::IMG_RECT(0, 0, 20, 20);
const AnimationData Glob::IMG{"res/projectiles/poison_glob_ss.png", 8, 100};

Glob::Glob(SDL_FPoint c, SDL_FPoint v)
    : mPos(std::make_shared<UIComponent>(Rect(), Elevation::PROJECTILES)),
      mV(v) {
    Rect imgR = IMG_RECT;
    imgR.setPos(c.x, c.y, Rect::Align::CENTER);
    mImg.set(IMG);
    mImg.setDest(imgR);
}

void Glob::init() {
    mResizeSub =
        ServiceSystem::Get<ResizeService, ResizeObservable>()->subscribe(
            [this](ResizeData d) { onResize(d); });
    mUpdateSub = TimeSystem::GetUpdateObservable()->subscribe(
        [this](Time dt) { onUpdate(dt); });
    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [this](SDL_Renderer* r) { onRender(r); }, mPos);
    mAnimTimerSub =
        ServiceSystem::Get<TimerService, TimerObservable>()->subscribe(
            [this](Timer& t) {
                mImg->nextFrame();
                return true;
            },
            IMG);
}

bool Glob::dead() const { return mDead; }

void Glob::setPos(float x, float y) {
    Rect imgR = mImg.getRect();
    imgR.setPos(x, y, Rect::Align::CENTER);
    mImg.setDest(imgR);
    mPos->rect = mImg.getDest();
}

void Glob::onResize(ResizeData data) {
    Rect imgR = mImg.getRect();
    imgR.moveFactor((float)data.newW / data.oldW, (float)data.newH / data.oldH);
    mImg.setDest(imgR);
    mPos->rect = mImg.getDest();
}

void Glob::onUpdate(Time dt) {
    float sec = dt.s();
    Rect imgD = mImg.getDest();
    imgD.move(mV.x * sec, mV.y * sec);

    SDL_Point dim = RenderSystem::getWindowSize();
    float x = imgD.x(), y = imgD.y();
    imgD.fitWithin(Rect(0, 0, dim.x, dim.y));
    if (x != imgD.x()) {
        mV.x *= -1;
    }
    if (y != imgD.y()) {
        mV.y *= -1;
    }

    HitObservable::Side side = GetHitObservable()->next(mPos->rect);

    if (side != HitObservable::Side::NONE) {
        onDeath();
        mDead = true;
        mResizeSub.reset();
        mRenderSub.reset();
        mUpdateSub.reset();
    } else {
        Rect imgR = mImg.getRect();
        imgR.setPos(imgD, Rect::Align::CENTER);
        mImg.setDest(imgR);
        mPos->rect = mImg.getDest();
    }
}

void Glob::onRender(SDL_Renderer* renderer) { TextureBuilder().draw(mImg); }

void Glob::onDeath() {}
