#ifndef TARGET_SYSTEM_H
#define TARGET_SYSTEM_H

#include <ServiceSystem/Observable.h>
#include <Wizards/WizardIds.h>

namespace TargetSystem {
// Observable for triggering events for a specific wizard
template <class T, class... Ts>
class TargetObservable : public ObservableBase {
   public:
    typedef std::function<void(const T&, const Ts&...)> IdSubscriptionFunc;
    typedef std::function<void(WizardId, const T&, const Ts&...)>
        AllSubscriptionFunc;

    class IdObservable
        : public Observable<void(const T&, const Ts&...), WizardId> {
        friend class TargetObservable<T, Ts...>;
    };
    class AllObservable
        : public Observable<void(WizardId, const T&, const Ts&...)> {
        friend class TargetObservable<T, Ts...>;
    };

    typedef typename IdObservable::SubscriptionPtr IdSubscriptionPtr;
    typedef typename AllObservable::SubscriptionPtr AllSubscriptionPtr;

    enum : size_t { FUNC = 0, DATA };

    virtual ~TargetObservable() = default;

    IdSubscriptionPtr subscribe(IdSubscriptionFunc func, WizardId id) {
        return mIdObservable.subscribe(func, id);
    }

    AllSubscriptionPtr subscribeToAll(AllSubscriptionFunc func) {
        return mAllObservable.subscribe(func);
    }

    void next(WizardId target, const T& t, const Ts&... args) {
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

template <class T>
class TargetDataObservable : public TargetObservable<T> {
   public:
    using typename TargetObservable<T>::IdSubscriptionFunc;
    using typename TargetObservable<T>::AllSubscriptionFunc;
    using typename TargetObservable<T>::IdSubscriptionPtr;
    using typename TargetObservable<T>::AllSubscriptionPtr;

    IdSubscriptionPtr subscribe(IdSubscriptionFunc func, WizardId id) {
        func(mData[id]);
        return TargetObservable<T>::subscribe(func, id);
    }

    AllSubscriptionPtr subscribeToAll(AllSubscriptionFunc func) {
        for (auto pair : mData) {
            func(pair.first, pair.second);
        }
        return TargetObservable<T>::subscribeToAll(func);
    }

    void next(WizardId target, const T& t) {
        mData[target] = t;
        TargetObservable<T>::next(target, t);
    }

    const T& get(WizardId target) { return mData[target]; }

   private:
    std::unordered_map<WizardId, T> mData;
};
}  // namespace TargetSystem

#endif
