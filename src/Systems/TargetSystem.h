#ifndef TARGET_SYSTEM_H
#define TARGET_SYSTEM_H

#include <ServiceSystem/Observable.h>
#include <Wizards/WizardIds.h>

namespace TargetSystem {
// Observable for triggering events for a specific wizard
template <class T, class... Ts>
class TargetObservable : public ObservableBase {
   public:
    class IdObservable : public Observable<void(T, Ts...), WizardId> {
        friend class TargetObservable<T, Ts...>;
    };
    class AllObservable : public Observable<void(WizardId, T, Ts...)> {
        friend class TargetObservable<T, Ts...>;
    };

    typedef typename IdObservable::SubscriptionPtr IdSubscriptionPtr;
    typedef typename AllObservable::SubscriptionPtr AllSubscriptionPtr;

    enum : size_t { FUNC = 0, DATA };

    IdSubscriptionPtr subscribe(std::function<void(T, Ts...)> func,
                                WizardId id) {
        return mIdObservable.subscribe(func, id);
    }

    AllSubscriptionPtr subscribeToAll(
        std::function<void(WizardId, T, Ts...)> func) {
        return mAllObservable.subscribe(func);
    }

    void next(WizardId target, T t, Ts... args) {
        for (auto sub : mIdObservable) {
            if (sub->template get<DATA>() == target) {
                sub->template get<FUNC>()(t, args...);
            }
        }
        for (auto sub : mAllObservable) {
            sub->template get<FUNC>()(target, t, args...);
        }
    }

   private:
    IdObservable mIdObservable;
    AllObservable mAllObservable;
};
}  // namespace TargetSystem

#endif
