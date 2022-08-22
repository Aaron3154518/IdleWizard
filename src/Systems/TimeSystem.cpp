#include "TimeSystem.h"

namespace TimeSystem {
// UpdateObservable
void UpdateObservable::init() {
    mUpdateSub =
        ServiceSystem::Get<::UpdateService, ::UpdateObservable>()->subscribe(
            [this](Time dt) { onUpdate(dt); });
}

void UpdateObservable::onUpdate(Time dt) {
    if (!isLocked()) {
        next(dt);
    }
}

std::shared_ptr<UpdateObservable> GetUpdateObservable() {
    return ServiceSystem::Get<UpdateService, UpdateObservable>();
}

// TimerObservable
TimerObservable::UpdateSubPtr TimerObservable::getUpdateSub(
    std::function<void(Time)> func) {
    return ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
        func);
}

std::shared_ptr<TimerObservable> GetTimerObservable() {
    return ServiceSystem::Get<TimerService, TimerObservable>();
}

// FreezeObservable
void FreezeObservable::freeze(FreezeType type) {
    if (!frozen(type)) {
        mLocks[type] = GetUpdateObservable()->requestLock();
        for (auto sub : *this) {
            sub->get<ON_FREEZE>()(type);
        }
    }
}
void FreezeObservable::unfreeze(FreezeType type) {
    auto it = mLocks.find(type);
    if (it != mLocks.end()) {
        GetUpdateObservable()->releaseLock(it->second);
        mLocks.erase(it);
        for (auto sub : *this) {
            sub->get<ON_UNFREEZE>()(type);
        }
    }
}
bool FreezeObservable::frozen(FreezeType type) const {
    return mLocks.find(type) != mLocks.end();
}

std::shared_ptr<FreezeObservable> GetFreezeObservable() {
    return ServiceSystem::Get<FreezeService, FreezeObservable>();
}

void Freeze(FreezeType type) { GetFreezeObservable()->freeze(type); }

void Unfreeze(FreezeType type) { GetFreezeObservable()->unfreeze(type); }

bool Frozen(FreezeType type) { return GetFreezeObservable()->frozen(type); }

}  // namespace TimeSystem
