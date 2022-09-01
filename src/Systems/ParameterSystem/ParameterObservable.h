#ifndef PARAMETER_OBSERVABLE_H
#define PARAMETER_OBSERVABLE_H

#include <ServiceSystem/Observable.h>
#include <Systems/ParameterSystem/WizardStates.h>
#include <Systems/WizardSystem.h>
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
    using ValueObservable::set;

    void init(const Number& defVal);
    void init(WizardSystem::Event event);
    void init(const Number& defVal, WizardSystem::Event event);

   private:
    Number mDefault = 0;
    WizardSystem::Event mResetEvent = WizardSystem::Event::NoReset;
    WizardSystem::WizardEventObservable::IdSubscriptionPtr mResetSub;
};

typedef std::shared_ptr<BaseValueObservable> BaseValueObservablePtr;

class BaseStateObservable : public StateObservable {
   public:
    using StateObservable::set;

    void init(bool defVal);
    void init(WizardSystem::Event event);
    void init(bool defVal, WizardSystem::Event event);

   private:
    bool mDefault = false;
    WizardSystem::Event mResetEvent = WizardSystem::Event::NoReset;
    WizardSystem::WizardEventObservable::IdSubscriptionPtr mResetSub;
};

typedef std::shared_ptr<BaseStateObservable> BaseStateObservablePtr;
}  // namespace ParameterSystem

#endif
