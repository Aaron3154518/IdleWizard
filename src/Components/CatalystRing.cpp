#include "CatalystRing.h"

namespace CatalystRing {
// HitObservable
void HitObservable::setPos(const CircleData& pos) { mPos = pos; }

void HitObservable::init() {
    mTimerSub = TimeSystem::GetTimerObservable()->subscribe(
        [this](Timer& t) { return onTimer(t); },
        [this](Time dt, Timer& t) { onTimerUpdate(dt, t); }, Timer(1000));
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
                sub->get<FUNC>()(params[CatalystParams::MagicEffect].get());
                std::cerr << "Shoot fireball" << std::endl;
            }
            mReady = false;
        }
    }
}

std::vector<HitObservable::SubscriptionWPtr> HitObservable::getInRange() {
    std::vector<SubscriptionWPtr> inRange;
    for (auto sub : *this) {
        auto& pos = sub->get<DATA>();
        float dx = mPos.c.x - pos->rect.cX(), dy = mPos.c.y - pos->rect.cY();
        float mag = sqrtf(dx * dx + dy * dy);
        if (mag < mPos.r2) {
            inRange.push_back(sub);
        }
    }
    return inRange;
}

std::shared_ptr<HitObservable> GetHitObservable() {
    return ServiceSystem::Get<Service, HitObservable>();
}
}  // namespace CatalystRing
