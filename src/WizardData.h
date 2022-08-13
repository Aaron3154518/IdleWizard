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

    template <WizardId id, WizardId... ids>
    ParameterSubscriptionPtr subscribe(
        std::function<void()> func, std::initializer_list<WizardType<id>> keys,
        std::initializer_list<WizardType<ids>>... tail) {
        ParameterSubscriptionPtr sub = subscribe<id>(keys, func);
        subscribe<ids...>(sub, tail...);
        return sub;
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
    void subscribe(std::initializer_list<WizardType<id>> keys,
                   ParameterSubscriptionPtr sub) {
        for (auto key : keys) {
            _observable<id>(key).subscribe(sub);
        }
    }

    template <WizardId...>
    void subscribe(ParameterSubscriptionPtr sub) {}

    template <WizardId id, WizardId... ids>
    void subscribe(ParameterSubscriptionPtr sub,
                   std::initializer_list<WizardType<id>> keys,
                   std::initializer_list<WizardType<ids>>... tail) {
        subscribe<id>(keys, sub);
        subscribe<ids...>(sub, tail...);
    }

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

#endif
