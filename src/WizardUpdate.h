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

typedef WizardUpdateObservable<WizardParams> WizardParameters;

class WizardUpdateService : public Service<WizardParameters> {};

#endif
