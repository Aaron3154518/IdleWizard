#ifndef STATE_SERVICE_H
#define STATE_SERVICE_H

#include <Systems/ParameterSystem/StateObservable.h>

#include <functional>
#include <memory>
#include <unordered_map>

namespace ParameterSystem {
template <class T>
class StateObservableMap : public ObservableBase {
   public:
    const StateObservablePtr& get(T key) {
        auto& result = mStates[key];
        if (!result) {
            result = std::make_unique<StateObservable>();
        }
        return result;
    }

    StateSubscriptionPtr subscribe(const std::initializer_list<T>& keys,
                                   std::function<void()> func) {
        StateSubscriptionPtr sub;
        for (T key : keys) {
            if (!sub) {
                sub = get(key)->subscribe(func);
            } else {
                get(key)->subscribe(sub);
            }
        }
        return sub;
    }

   private:
    std::unordered_map<T, StateObservablePtr> mStates;
};

template <class ServiceT, class T>
std::shared_ptr<StateObservableMap<T>> Get() {
    return ServiceSystem::Get<ServiceT, StateObservableMap<T>>();
}

template <class ServiceT, class T>
struct StateAccess {
    static const bool& get(T key) {
        return Get<ServiceT, T>()->get(key)->get();
    }

    static void set(T key, const bool& val) {
        Get<ServiceT, T>()->get(key)->set(val);
    }

    static StateSubscriptionPtr subscribe(T key,
                                          std::function<void(bool)> func) {
        return Get<ServiceT, T>()->get(key)->subscribe(
            [func, key]() { func(get(key)); });
    }

    static StateSubscriptionPtr subscribe(const std::initializer_list<T>& keys,
                                          std::function<void()> func) {
        return Get<ServiceT, T>()->subscribe(keys, func);
    }
};
}  // namespace ParameterSystem

#endif
