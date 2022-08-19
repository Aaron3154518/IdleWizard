#ifndef PARAMETER_OBSERVABLE_H
#define PARAMETER_OBSERVABLE_H

#include <ServiceSystem/Observable.h>
#include <Systems/ParameterSystem/WizardParams.h>
#include <Utils/Number.h>

namespace ParameterSystem {
typedef ForwardObservable<void()> ParameterObservableBase;
class ParameterObservable : public ParameterObservableBase {
   public:
    const Number& get() const;

    void set(const Number& val);

   private:
    void onSubscribe(SubscriptionPtr sub);

    Number mVal;
};
typedef ParameterObservable::SubscriptionPtr ParameterSubscriptionPtr;
}  // namespace ParameterSystem

#endif
