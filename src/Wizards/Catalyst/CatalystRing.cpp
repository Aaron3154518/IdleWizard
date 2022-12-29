#include "CatalystRing.h"

namespace Catalyst {
namespace Ring {
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
    std::function<void()> func, UIComponentPtr pos) {
    return subscribe([func](bool b) { func(); }, pos);
}
HitObservable::SubscriptionPtr HitObservable::subscribe(
    std::function<void(bool)> func, UIComponentPtr pos) {
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

    mNumZapsSub = Catalyst::Params::get(Catalyst::Param::ZapperUp)
                      .subscribe([this](const Number& numZaps) {
                          mMaxZaps = std::max(numZaps.toInt(), 1);
                          mReady = mMaxZaps;
                          mTimerSub->get<TimerObservable::DATA>().setLength(
                              1000 / mMaxZaps);
                      });
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
    mReady = std::min(++mReady, mMaxZaps);
    return true;
}

void HitObservable::onTimerUpdate(Time dt, Timer& timer) {
    if (mReady > 0) {
        auto inRange = getInRange();
        if (!inRange.empty()) {
            int idx = (int)(rDist(gen) * inRange.size());
            auto sub = inRange.at(idx).lock();
            if (sub) {
                auto poisCntParam = Catalyst::Params::get(
                    Catalyst::Param::CatRingPoisCnt);
                int poisCnt = poisCntParam.get().toInt();

                sub->get<ZAP_CNT>()++;
                sub->get<FUNC>()(poisCnt > 0);
                Rect fBallRect = sub->get<DATA>()->rect;
                mZaps.push_back(std::move(ComponentFactory<Zap>::New(
                    SDL_Point{fBallRect.CX(), fBallRect.CY()}, mCircle.c)));

                if (poisCnt > 0) {
                    poisCntParam.set(poisCnt - 1);
                }
                mReady--;
            }
        }
    }
}

std::vector<HitObservable::SubscriptionWPtr> HitObservable::getInRange() {
    std::vector<SubscriptionWPtr> inRange;
    int zapCnt = Catalyst::Params::get(Catalyst::Param::ZapCntUp)
                     .get()
                     .toInt();
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
}  // namespace Ring
}  // namespace Catalyst
