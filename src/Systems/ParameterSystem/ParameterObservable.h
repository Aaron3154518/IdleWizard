#ifndef PARAMETER_OBSERVABLE_H
#define PARAMETER_OBSERVABLE_H

#include <ServiceSystem/Observable.h>
#include <Systems/ParameterSystem/WizardStates.h>
#include <Utils/Number.h>

#include <initializer_list>
#include <list>
#include <memory>

namespace ParameterSystem {
typedef ForwardObservable<void()> ParamObservableBase;

typedef std::shared_ptr<ParamObservableBase> ParamObservableBasePtr;
typedef ParamObservableBase::SubscriptionPtr ParameterSubscriptionPtr;
typedef ParamObservableBase::SubscriptionWPtr ParameterSubscriptionWPtr;

class ValueObservable : public ParamObservableBase {
   public:
    ~ValueObservable() = default;

    const Number& get() const;

   protected:
    void set(const Number& val);

   private:
    Number mVal;
};

typedef std::shared_ptr<ValueObservable> ValueObservablePtr;

class StateObservable : public ParamObservableBase {
   public:
    ~StateObservable() = default;

    bool get() const;

   protected:
    void set(bool state);

   private:
    bool mState;
};

typedef std::shared_ptr<StateObservable> StateObservablePtr;

class NodeValueObservable : public ValueObservable {
    template <WizardId>
    friend class Params;
    friend class NodeValue;
};

typedef std::shared_ptr<NodeValueObservable> NodeValueObservablePtr;

class NodeStateObservable : public StateObservable {
    friend class NodeState;
};

typedef std::shared_ptr<NodeStateObservable> NodeStateObservablePtr;

class BaseValueObservable : public ValueObservable {
   public:
    BaseValueObservable();

    using ValueObservable::set;

    void setResetTier(ResetTier tier);

    void setDefault(const Number& val);

    Number mDefault;

   private:
    SubscriptionPtr mResetSub;
    ResetTier mResetTier = ResetTier::T1;
};

typedef std::shared_ptr<BaseValueObservable> BaseValueObservablePtr;

class BaseStateObservable : public StateObservable {
   public:
    BaseStateObservable();

    using StateObservable::set;

    void setResetTier(ResetTier tier);

    void setDefault(bool state);

    bool mDefault;

   private:
    SubscriptionPtr mResetSub;
    ResetTier mResetTier = ResetTier::T1;
};

typedef std::shared_ptr<BaseStateObservable> BaseStateObservablePtr;
}  // namespace ParameterSystem

#endif
