#include "CatalystRing.h"

namespace CatalystRing {
// HitObservable::Zap
HitObservable::Zap::Zap(SDL_Point p1, SDL_Point p2) : mP1(p1), mP2(p2) {}

bool HitObservable::Zap::dead() const { return !mTimerSub; }

void HitObservable::Zap::init() {
    mTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) { return onTimer(t); }, Timer(500));
}

bool HitObservable::Zap::onTimer(Timer& timer) {
    mTimerSub.reset();
    return false;
}

// HitObservable
const std::string HitObservable::ZAP_IMG = "res/projectiles/catalyst_zap.png";

HitObservable::HitObservable()
    : mPos(std::make_shared<UIComponent>(Rect(), Elevation::CATALYST_ZAP)) {
    mPos->mouse = false;
}

HitObservable::SubscriptionPtr HitObservable::subscribe(
    std::function<void(const Number&)> func, UIComponentPtr pos) {
    return HitObservableBase::subscribe(func, pos, 0);
}

void HitObservable::setPos(const CircleData& circle) { mCircle = circle; }

void HitObservable::init() {
    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [this](SDL_Renderer* r) { onRender(r); }, mPos);
    mTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) { return onTimer(t); },
        [this](Time dt, Timer& t) { onTimerUpdate(dt, t); }, Timer(1000));
    mCatHideSub = WizardSystem::GetHideObservable()->subscribe(
        [this](bool hide) {
            mTimerSub->setActive(!hide);
            mZaps.clear();
        },
        CATALYST);
}

void HitObservable::onRender(SDL_Renderer* r) {
    for (auto it = mZaps.begin(); it != mZaps.end();) {
        if ((*it)->dead()) {
            it = mZaps.erase(it);
        } else {
            SDL_RenderDrawLine(r, (*it)->mP1.x, (*it)->mP1.y, (*it)->mP2.x,
                               (*it)->mP2.y);
            ++it;
        }
    }
}

bool HitObservable::onTimer(Timer& timer) {
    mReady = true;
    return true;
}

void HitObservable::onTimerUpdate(Time dt, Timer& timer) {
    if (mReady) {
        auto inRange = getInRange();
        if (!inRange.empty()) {
            int idx = (int)(rDist(gen) * inRange.size());
            auto sub = inRange.at(idx).lock();
            if (sub) {
                ParameterSystem::Params<CATALYST> params;
                sub->get<ZAP_CNT>()++;
                sub->get<FUNC>()(params[CatalystParams::MagicEffect].get());
                Rect fBallRect = sub->get<DATA>()->rect;
                mZaps.push_back(std::move(ComponentFactory<Zap>::New(
                    SDL_Point{fBallRect.CX(), fBallRect.CY()}, mCircle.c)));
            }
            mReady = false;
        }
    }
}

std::vector<HitObservable::SubscriptionWPtr> HitObservable::getInRange() {
    std::vector<SubscriptionWPtr> inRange;
    int zapCnt =
        ParameterSystem::Param<CATALYST>(CatalystParams::ZapCnt).get().toInt();
    for (auto sub : *this) {
        if (sub->get<ZAP_CNT>() >= zapCnt) {
            continue;
        }

        auto& pos = sub->get<DATA>();
        float dx = mCircle.c.x - pos->rect.cX(),
              dy = mCircle.c.y - pos->rect.cY();
        float mag = sqrtf(dx * dx + dy * dy);
        if (mag < mCircle.r2) {
            inRange.push_back(sub);
        }
    }
    return inRange;
}

std::shared_ptr<HitObservable> GetHitObservable() {
    return ServiceSystem::Get<Service, HitObservable>();
}
}  // namespace CatalystRing
