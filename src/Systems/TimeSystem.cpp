#include "TimeSystem.h"

namespace TimeSystem {
// UpdateObservable
void UpdateObservable::init() {
    mUpdateSub =
        ServiceSystem::Get<::UpdateService, ::UpdateObservable>()->subscribe(
            std::bind(&UpdateObservable::onUpdate, this,
                      std::placeholders::_1));
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
}  // namespace TimeSystem
