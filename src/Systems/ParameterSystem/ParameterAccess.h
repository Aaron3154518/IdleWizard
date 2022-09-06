#ifndef PARAMETER_ACCESS_H
#define PARAMETER_ACCESS_H

#include <Systems/ParameterSystem/ParameterDag.h>
#include <Systems/ParameterSystem/ParameterObservable.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Systems/ParameterSystem/WizardStates.h>
#include <Utils/Number.h>
#include <Wizards/WizardIds.h>

#include <functional>
#include <initializer_list>
#include <list>
#include <memory>

namespace ParameterSystem {
// Base class for different Value templates
struct ValueParam {
   public:
    ValueParam(const ValueParam& other);
    virtual ~ValueParam() = default;

    ValueParam& operator=(const ValueParam& other);
    bool operator==(const ValueParam& other) const;

    ValueObservablePtr operator->() const;

    const Number& get() const;

    ParameterSubscriptionPtr subscribe(std::function<void()> func,
                                       bool fire = true) const;
    ParameterSubscriptionPtr subscribe(std::function<void(const Number&)> func,
                                       bool fire = true) const;

    const WizardId mId;
    const param_t mKey;
    const bool mIsBase;

   protected:
    ValueParam(WizardId id, param_t key, bool isBase);
};

typedef std::unique_ptr<ValueParam> ValueParamPtr;

// Base class for different State templates
struct StateParam {
   public:
    StateParam(const StateParam& other);
    virtual ~StateParam() = default;

    StateParam& operator=(const StateParam& other);
    bool operator==(const StateParam& other) const;

    StateObservablePtr operator->() const;

    bool get() const;

    ParameterSubscriptionPtr subscribe(std::function<void()> func,
                                       bool fire = true) const;
    ParameterSubscriptionPtr subscribe(std::function<void(bool)> func,
                                       bool fire = true) const;

    const param_t mKey;
    const bool mIsBase;

   protected:
    StateParam(param_t key, bool isBase);
};

typedef std::unique_ptr<StateParam> StateParamPtr;

// Classes to base params
struct BaseValue : public ValueParam {
    template <WizardId id>
    friend BaseValue Param(WizardBaseType<id> key);

   public:
    BaseValueObservablePtr operator->() const;

    void set(const Number& val) const;

   private:
    BaseValue(WizardId id, param_t key);
};

typedef std::unique_ptr<BaseValue> BaseValuePtr;

struct BaseState : public StateParam {
    friend BaseState Param(State::B key);

   public:
    BaseStateObservablePtr operator->() const;

    void set(bool state) const;

   private:
    BaseState(param_t key);
};

typedef std::unique_ptr<BaseState> BaseStatePtr;

// Classes for node params
struct NodeValue : public ValueParam {
    template <WizardId id>
    friend NodeValue Param(WizardNodeType<id> key);

   public:
    NodeValueObservablePtr operator->() const;

    ParameterSubscriptionPtr subscribeTo(
        const std::initializer_list<ValueParam>& values,
        const std::initializer_list<StateParam>& states,
        std::function<Number()> func, bool fire = true) const;
    ParameterSubscriptionPtr subscribeTo(
        ValueParam param, std::function<Number(const Number&)> func,
        bool fire = true) const;
    ParameterSubscriptionPtr subscribeTo(StateParam param,
                                         std::function<Number(bool)> func,
                                         bool fire = true) const;

   private:
    NodeValue(WizardId id, param_t key);
};

typedef std::unique_ptr<NodeValue> NodeValuePtr;

struct NodeState : public StateParam {
    friend NodeState Param(State::N key);

   public:
    NodeStateObservablePtr operator->() const;

    ParameterSubscriptionPtr subscribeTo(
        const std::initializer_list<ValueParam>& values,
        const std::initializer_list<StateParam>& states,
        std::function<bool()> func, bool fire = true) const;
    ParameterSubscriptionPtr subscribeTo(
        ValueParam param, std::function<bool(const Number&)> func,
        bool fire = true) const;
    ParameterSubscriptionPtr subscribeTo(StateParam param,
                                         std::function<bool(bool)> func,
                                         bool fire = true) const;

   private:
    NodeState(param_t key);
};

typedef std::unique_ptr<NodeState> NodeStatePtr;

// Creates a param
template <WizardId id>
BaseValue Param(WizardBaseType<id> key) {
    return BaseValue(id, key);
}

template <WizardId id>
NodeValue Param(WizardNodeType<id> key) {
    return NodeValue(id, key);
}

BaseState Param(State::B key);

NodeState Param(State::N key);

// Subscribes to multiple params
ParameterSubscriptionPtr subscribe(
    const std::initializer_list<ValueParam>& values,
    const std::initializer_list<StateParam>& states, std::function<void()> func,
    bool fire = true);

// Provides easy access to params from one wizard
template <WizardId id>
struct Params {
    BaseValue operator[](WizardBaseType<id> key) { return Param<id>(key); }

    NodeValue operator[](WizardNodeType<id> key) { return Param<id>(key); }
};

// Provides easy access to state params
struct States {
    BaseState operator[](State::B key);

    NodeState operator[](State::N key);
};
}  // namespace ParameterSystem

#endif
