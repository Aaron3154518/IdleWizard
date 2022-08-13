#ifndef WIZARD_UPDATE_H
#define WIZARD_UPDATE_H

#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Utils/Number.h>

#include <unordered_map>

#include "WizardIds.h"

template <class K>
using ParameterList = std::unordered_map<K, Number>;

template <class K>
using WizardUpdateObservableBase =
    ForwardObservable<void(const ParameterList<K>&)>;

template <class K>
class WizardUpdateObservable : public WizardUpdateObservableBase<K>,
                               public Component {
   public:
    using WizardUpdateObservableBase<K>::subscribe;
    using WizardUpdateObservableBase<K>::next;

    WizardUpdateObservable() = default;

    void setParam(K key, Number val) { mParams[key] = val; }
    const Number& getParam(K key, Number defVal) {
        auto it = mParams.find(key);
        return it != mParams.end() ? it->second : defVal;
    }

   private:
    void init() {
        mUpdateSub =
            ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
                std::bind(&WizardUpdateObservable::onUpdate, this,
                          std::placeholders::_1));
    }

    void onUpdate(Time dt) { next(mParams); }

    UpdateObservable::SubscriptionPtr mUpdateSub;

    ParameterList<K> mParams;
};

template <class K>
class WizardUpdateService : public Service<WizardUpdateObservable<K>> {};

namespace WizardParameters {
template <class K>
using SubscriptionPtr = typename WizardUpdateObservable<K>::SubscriptionPtr;

template <class K>
const Number& Get(K key, Number defVal = 0) {
    return ServiceSystem::Get<WizardUpdateService<K>,
                              WizardUpdateObservable<K>>()
        ->getParam(key, defVal);
}

template <class K>
void Set(K key, Number val) {
    ServiceSystem::Get<WizardUpdateService<K>, WizardUpdateObservable<K>>()
        ->setParam(key, val);
}

template <class K>
SubscriptionPtr<K> Subscribe(
    std::function<void(const ParameterList<K>&)> func) {
    return ServiceSystem::Get<WizardUpdateService<K>,
                              WizardUpdateObservable<K>>()
        ->subscribe(func);
}
}  // namespace WizardParameters

#endif
