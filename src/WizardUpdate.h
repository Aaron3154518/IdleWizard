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
    Observable<const ParameterList<K>&, void(const ParameterList<K>&)>;

template <class K>
class WizardUpdateObservable : public WizardUpdateObservableBase<K>,
                               public Component {
   public:
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
        mUpdateSub->setUnsubscriber(unsub);
    }

    void onUpdate(Time dt) { WizardUpdateObservableBase<K>::next(mParams); }

    UpdateObservable::SubscriptionPtr mUpdateSub;

    ParameterList<K> mParams;
};

typedef WizardUpdateObservable<WizardParams> WizardParameters;

class WizardUpdateService : public Service<WizardParameters> {};

#endif
