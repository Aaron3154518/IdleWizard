#ifndef WIZARD_UPDATE_H
#define WIZARD_UPDATE_H

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

class ParameterMap : public ObservableBase {
   public:
    template <class K>
    ParameterSubscriptionPtr subscribe(WizardId id, K key,
                                       std::function<void()> func) {
        func();
        return _observable(id, key).subscribe(func);
    }

    template <class K>
    ParameterSubscriptionPtr subscribe(WizardId id,
                                       std::initializer_list<K> keys,
                                       std::function<void()> func) {
        ParameterSubscriptionPtr sub;
        for (K key : keys) {
            if (!sub) {
                sub = subscribe(id, key, func);
            } else {
                _observable(id, key).subscribe(sub);
            }
        }
        return sub;
    }

    template <class K, class... Ks>
    ParameterSubscriptionPtr subscribe(
        std::function<void()> func,
        std::pair<WizardId, std::initializer_list<K>> keys,
        std::pair<WizardId, std::initializer_list<Ks>>... tail) {
        ParameterSubscriptionPtr sub = subscribe(keys.first, keys.second, func);
        subscribe<Ks...>(sub, tail...);
        return sub;
    }

    template <class K>
    const Number& get(WizardId id, K key) {
        return _parameter(id, key);
    }

    template <class K>
    void set(WizardId id, K key, const Number& val) {
        _parameter(id, key) = val;
        _observable(id, key).next();
    }

   private:
    template <class K>
    void subscribe(WizardId id, std::initializer_list<K> keys,
                   ParameterSubscriptionPtr sub) {
        for (K key : keys) {
            _observable(id, key).subscribe(sub);
        }
    }

    template <class... Ks>
    void subscribe(ParameterSubscriptionPtr sub) {}

    template <class K, class... Ks>
    void subscribe(ParameterSubscriptionPtr sub,
                   std::pair<WizardId, std::initializer_list<K>> keys,
                   std::pair<WizardId, std::initializer_list<Ks>>... tail) {
        subscribe(keys.first, keys.second, sub);
        subscribe<Ks...>(sub, tail...);
    }

    template <class K>
    ParameterList<K>& _list(WizardId id, K key) {
        if (typeid(K) != WIZ_DATA_TYPES.at(id)) {
            throw std::runtime_error(
                "ParameterMap::get(): " + WIZ_NAMES.at(id) +
                " expected key of type " +
                std::string(WIZ_DATA_TYPES.at(id).name()) +
                " but received key of type " + std::string(typeid(K).name()));
        }
        if (mParams.find(id) == mParams.end()) {
            mParams[id] = std::make_unique<ParameterList<K>>();
        }
        return dynamic_cast<ParameterList<K>&>(*mParams[id]);
    }

    template <class K>
    Number& _parameter(WizardId id, K key) {
        return _list(id, key).mParameters[key];
    }

    template <class K>
    ParameterObservable& _observable(WizardId id, K key) {
        return _list(id, key).mObservables[key];
    }

    std::unordered_map<WizardId, ParameterListPtr> mParams;
};

class ParameterService : public Service<ParameterMap> {};

#endif
