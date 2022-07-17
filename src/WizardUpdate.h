#ifndef WIZARD_UPDATE_H
#define WIZARD_UPDATE_H

#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Utils/Number.h>

#include <unordered_map>
typedef std::unordered_map<int, Number> ParameterList;

typedef Observable<const ParameterList&, void(const ParameterList&)>
    WizardUpdateObservableBase;

class WizardUpdateObservable : public WizardUpdateObservableBase,
                               public Component {
   public:
    WizardUpdateObservable() = default;

    void setParam(int key, Number val);
    const Number& getParam(int key, Number defVal);

   private:
    void init();

    void onUpdate(Time dt);

    UpdateObservable::SubscriptionPtr mUpdateSub;

    ParameterList mParams;
};

class WizardUpdateService : public Service<WizardUpdateObservable> {};

#endif
