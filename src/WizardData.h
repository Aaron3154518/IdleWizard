#ifndef WIZARD_DATA_H
#define WIZARD_DATA_H

#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Utils/Number.h>

#include <initializer_list>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "WizardIds.h"
#include "WizardTypes.h"

typedef ForwardObservable<void()> ParameterObservableBase;
class ParameterObservable : public ParameterObservableBase {};
typedef ParameterObservable::SubscriptionPtr ParameterSubscriptionPtr;

struct ParameterListBase {
    virtual ~ParameterListBase() = default;
};
typedef std::unique_ptr<ParameterListBase> ParameterListPtr;
template <class K>
struct ParameterList : public ParameterListBase {
    std::unordered_map<K, ParameterObservable> mObservables;
    std::unordered_map<K, Number> mParameters;
};

// Forward declarations
class ParameterMap;

// Helper classes for subscribing keys
template <WizardId id, WizardType<id>... keys>
struct Keys {};

template <class...>
struct KeySubscriber;

template <>
struct KeySubscriber<> {
    static void subscribe(ParameterMap* map, ParameterSubscriptionPtr sub) {}

    static ParameterSubscriptionPtr subscribe(ParameterMap* map,
                                              std::function<void()> func) {
        return nullptr;
    }
};

template <WizardId id, WizardType<id>... keys, class... Tail>
struct KeySubscriber<Keys<id, keys...>, Tail...>
    : public KeySubscriber<Tail...> {
    static void subscribe(ParameterMap* map, ParameterSubscriptionPtr sub);

    static ParameterSubscriptionPtr subscribe(ParameterMap* map,
                                              std::function<void()> func);
};

// Stores numbers by wizard id and param enum
class ParameterMap : public ObservableBase {
   public:
    template <WizardId id>
    ParameterSubscriptionPtr subscribe(WizardType<id> key,
                                       std::function<void()> func) {
        func();
        return _observable<id>(key).subscribe(func);
    }

    template <WizardId id>
    ParameterSubscriptionPtr subscribe(
        std::initializer_list<WizardType<id>> keys,
        std::function<void()> func) {
        ParameterSubscriptionPtr sub;
        for (auto key : keys) {
            if (!sub) {
                sub = subscribe<id>(key, func);
            } else {
                _observable<id>(key).subscribe(sub);
            }
        }
        return sub;
    }

    template <WizardId id>
    void subscribe(std::initializer_list<WizardType<id>> keys,
                   ParameterSubscriptionPtr sub) {
        for (auto key : keys) {
            _observable<id>(key).subscribe(sub);
        }
    }

    template <class... Ts>
    ParameterSubscriptionPtr subscribe(std::function<void()> func) {
        return KeySubscriber<Ts...>::subscribe(this, func);
    }

    template <WizardId id>
    const Number& get(WizardType<id> key) {
        return _parameter<id>(key);
    }

    template <WizardId id>
    void set(WizardType<id> key, const Number& val) {
        _parameter<id>(key) = val;
        _observable<id>(key).next();
    }

   private:
    template <WizardId id>
    ParameterList<WizardType<id>>& _list(WizardType<id> key) {
        if (mParams.find(id) == mParams.end()) {
            mParams[id] = std::make_unique<ParameterList<WizardType<id>>>();
        }
        return dynamic_cast<ParameterList<WizardType<id>>&>(*mParams[id]);
    }

    template <WizardId id>
    Number& _parameter(WizardType<id> key) {
        return _list<id>(key).mParameters[key];
    }

    template <WizardId id>
    ParameterObservable& _observable(WizardType<id> key) {
        return _list<id>(key).mObservables[key];
    }

    std::unordered_map<WizardId, ParameterListPtr> mParams;
};

class ParameterService : public Service<ParameterMap> {};

std::shared_ptr<ParameterMap> Parameters();

// Implement KeySubscriber after defining ParameterMap
template <WizardId id, WizardType<id>... keys, class... Tail>
void KeySubscriber<Keys<id, keys...>, Tail...>::subscribe(
    ParameterMap* map, ParameterSubscriptionPtr sub) {
    map->subscribe<id>({keys...}, sub);
    KeySubscriber<Tail...>::subscribe(map, sub);
}

template <WizardId id, WizardType<id>... keys, class... Tail>
ParameterSubscriptionPtr KeySubscriber<Keys<id, keys...>, Tail...>::subscribe(
    ParameterMap* map, std::function<void()> func) {
    ParameterSubscriptionPtr sub = map->subscribe<id>({keys...}, func);
    KeySubscriber<Tail...>::subscribe(map, sub);
    return sub;
}

// Struct for storing params
struct ParamBase {
    virtual ~ParamBase() = default;

    virtual Number get() const;

    virtual void set(const Number& val) const;
};

template <WizardId id>
struct Param : public ParamBase {
    Param(WizardType<id> _key) : key(_key) {}
    Param(const Param<id>* other) : Param(other.key) {}

    Number get() const { return Parameters()->get<id>(key); }

    void set(const Number& val) const { Parameters()->set<id>(key, val); }

    WizardType<id> key;
    const static WizardId ID = id;
};

typedef std::shared_ptr<ParamBase> ParamBasePtr;

#endif