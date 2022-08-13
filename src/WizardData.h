#ifndef WIZARD_UPDATE_H
#define WIZARD_UPDATE_H

#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/UpdateService.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Utils/Number.h>

#include <typeindex>
#include <typeinfo>
#include <unordered_map>

#include "WizardBase.h"
#include "WizardIds.h"

struct WizardData {
    Number basePower = 1;
    Number power = basePower;
    Number powerUp = 0;
    Number speed = 1000;
};

struct CrystalData {
    Number magic = 0;
    Number magicEffect = 1;
};

struct CatalystData {
    Number magic = 0;
    Number magicEffect = 1;
    Number capacity = 100;
};

struct WizardsData {
    std::shared_ptr<const WizardData> wizard;
    std::shared_ptr<const CrystalData> crystal;
    std::shared_ptr<const CatalystData> catalyst;
};

typedef ForwardObservable<void(const WizardsData&, Time)>
    WizardsDataObservableBase;
class WizardsDataObservable : public WizardsDataObservableBase,
                              public Component {
   public:
    void set(WizardBase* wizard);

   private:
    void init();

    void onUpdate(Time dt);

    WizardsData mData;

    UpdateObservable::SubscriptionPtr mUpdateSub;
};

class WizardsDataService : public Service<WizardsDataObservable> {};

#endif
