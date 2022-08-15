#ifndef TIME_SERVICE_H
#define TIME_SERVICE_H

#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <ServiceSystem/Lockable.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <ServiceSystem/UpdateServices/TimerService.h>

#include <stack>

namespace TimeSystem {
class UpdateObservable : public ForwardObservable<void(Time)>,
                         public Component,
                         public Lockable {
   private:
    void init();

    void onUpdate(Time dt);

    ::UpdateObservable::SubscriptionPtr mUpdateSub;
};

class UpdateService : public Service<UpdateObservable> {};

std::shared_ptr<UpdateObservable> GetUpdateObservable();

class TimerObservable : public TimerObservableBase {
   private:
    UpdateSubPtr getUpdateSub(std::function<void(Time)> func);
};

class TimerService : public Service<TimerObservable> {};

std::shared_ptr<TimerObservable> GetTimerObservable();

enum FreezeType : size_t { TIME_WIZARD = 0 };

class FreezeObservable : public Observable<void(FreezeType), void(FreezeType)> {
   public:
    enum : size_t { ON_FREEZE = 0, ON_UNFREEZE };

    void freeze(FreezeType type);
    void unfreeze(FreezeType type);
    bool frozen(FreezeType type) const;

   private:
    std::unordered_map<FreezeType, Lock> mLocks;
};

class FreezeService : public Service<FreezeObservable> {};

std::shared_ptr<FreezeObservable> GetFreezeObservable();
void Freeze(FreezeType type);
void Unfreeze(FreezeType type);
bool Frozen(FreezeType type);
}  // namespace TimeSystem

#endif
