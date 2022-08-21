#ifndef STATE_OBSERVABLE_H
#define STATE_OBSERVABLE_H

#include <ServiceSystem/Observable.h>

#include <memory>

namespace ParameterSystem {
typedef ForwardObservable<void()> StateObservableBase;
class StateObservable : public StateObservableBase {
   public:
    const bool& get() const;

    void set(const bool& val);

   private:
    void onSubscribe(SubscriptionPtr sub);

    bool mVal = false;
};
typedef StateObservable::SubscriptionPtr StateSubscriptionPtr;

typedef std::unique_ptr<StateObservable> StateObservablePtr;
}  // namespace ParameterSystem

#endif
