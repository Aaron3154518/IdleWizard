#ifndef PARAMETER_ACCESS_H
#define PARAMETER_ACCESS_H

#include <Systems/ParameterSystem/ParameterDag.h>
#include <Systems/ParameterSystem/ParameterObservable.h>
#include <Utils/Number.h>
#include <Wizards/WizardIds.h>

#include <functional>
#include <initializer_list>
#include <list>
#include <memory>
#include <type_traits>
#include <utility>

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

    WizardId mId;
    key_t mKey;
    bool mIsBase;

   protected:
    ValueParam(WizardId id, key_t key, bool isBase);
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

    key_t mKey;
    bool mIsBase;

   protected:
    StateParam(key_t key, bool isBase);
};

typedef std::unique_ptr<StateParam> StateParamPtr;

// Classes to base params
struct BaseValue : public ValueParam {
    template <WizardId id, class BaseParamT, class NodeParamT, class BaseStateT,
              class NodeStateT>
    friend class Params;

   public:
    BaseValueObservablePtr operator->() const;

    void set(const Number& val) const;

   private:
    BaseValue(WizardId id, key_t key);
};

typedef std::unique_ptr<BaseValue> BaseValuePtr;

struct BaseState : public StateParam {
    template <WizardId id, class BaseParamT, class NodeParamT, class BaseStateT,
              class NodeStateT>
    friend class Params;

   public:
    BaseStateObservablePtr operator->() const;

    void set(bool state) const;

   private:
    BaseState(key_t key);
};

typedef std::unique_ptr<BaseState> BaseStatePtr;

// Classes for node params
struct NodeValue : public ValueParam {
    template <WizardId id, class BaseParamT, class NodeParamT, class BaseStateT,
              class NodeStateT>
    friend class Params;

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
    NodeValue(WizardId id, key_t key);
};

typedef std::unique_ptr<NodeValue> NodeValuePtr;

struct NodeState : public StateParam {
    template <WizardId id, class BaseParamT, class NodeParamT, class BaseStateT,
              class NodeStateT>
    friend class Params;

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
    NodeState(key_t key);
};

typedef std::unique_ptr<NodeState> NodeStatePtr;

// Subscribes to multiple params
ParameterSubscriptionPtr subscribe(
    const std::initializer_list<ValueParam>& values,
    const std::initializer_list<StateParam>& states, std::function<void()> func,
    bool fire = true);

// Creates parameters
template <WizardId id, class BaseParamT, class NodeParamT, class BaseStateT,
          class NodeStateT>
class Params {
    static_assert(std::is_convertible<BaseParamT, key_t>::value,
                  "Params: BaseParamT is not a valid key type");
    static_assert(std::is_convertible<NodeParamT, key_t>::value,
                  "Params: NodeParamT is not a valid key type");
    static_assert(std::is_convertible<BaseStateT, key_t>::value,
                  "Params: BaseStateT is not a valid key type");
    static_assert(std::is_convertible<NodeStateT, key_t>::value,
                  "Params: NodeStateT is not a valid key type");

   public:
    static BaseValue get(BaseParamT key) { return BaseValue(id, key); }
    BaseValue operator[](BaseParamT key) const { return get(key); }

    static NodeValue get(NodeParamT key) { return NodeValue(id, key); }
    NodeValue operator[](NodeParamT key) const { return get(key); }

    static BaseState get(BaseStateT key) { return BaseState(id, key); }
    BaseState operator[](BaseStateT key) const { return get(key); }

    static NodeState get(NodeStateT key) { return NodeState(id, key); }
    NodeState operator[](NodeStateT key) const { return get(key); }
};
}  // namespace ParameterSystem

#endif
