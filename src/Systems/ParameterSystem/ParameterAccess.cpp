#include "ParameterAccess.h"

namespace ParameterSystem {

// ValueParam
ValueParam::ValueParam(WizardId id, param_t key, bool isBase)
    : mId(id), mKey(key), mIsBase(isBase) {}
ValueParam::ValueParam(const ValueParam& other)
    : ValueParam(other.mId, other.mKey, other.mIsBase) {}
ValueParam& ValueParam::operator=(const ValueParam& other) {
    *this = ValueParam(other);
    return *this;
}

ValueObservablePtr ValueParam::getObservable() const {
    if (mIsBase) {
        return ParameterDag::GetBase(mId, mKey);
    }
    return ParameterDag::GetNode(mId, mKey);
}

const Number& ValueParam::get() const { return getObservable()->get(); }

ParameterSubscriptionPtr ValueParam::subscribe(
    std::function<void(const Number&)> func) const {
    auto id = mId;
    auto key = mKey;
    if (mIsBase) {
        return getObservable()->subscribe(
            [id, key, func]() { func(ParameterDag::GetBase(id, key)->get()); });
    }
    return getObservable()->subscribe(
        [id, key, func]() { func(ParameterDag::GetNode(id, key)->get()); });
}

// StateParam
StateParam::StateParam(param_t key, bool isBase) : mKey(key), mIsBase(isBase) {}
StateParam::StateParam(const StateParam& other)
    : StateParam(other.mKey, other.mIsBase) {}
StateParam& StateParam::operator=(const StateParam& other) {
    *this = StateParam(other);
    return *this;
}

StateObservablePtr StateParam::getObservable() const {
    if (mIsBase) {
        return ParameterDag::GetBase(mKey);
    }
    return ParameterDag::GetNode(mKey);
}

bool StateParam::get() const { return getObservable()->get(); }

ParameterSubscriptionPtr StateParam::subscribe(
    std::function<void(bool)> func) const {
    auto key = mKey;
    if (mIsBase) {
        return getObservable()->subscribe(
            [key, func]() { func(ParameterDag::GetBase(key)->get()); });
    }
    return getObservable()->subscribe(
        [key, func]() { func(ParameterDag::GetNode(key)->get()); });
}

// BaseValue
BaseValue::BaseValue(WizardId id, param_t key) : ValueParam(id, key, true) {}

BaseValueObservablePtr BaseValue::getObservable() const {
    return ParameterDag::GetBase(mId, mKey);
}

void BaseValue::set(const Number& val) const { getObservable()->set(val); }

// BaseState
BaseState::BaseState(param_t key) : StateParam(key, true) {}

BaseStateObservablePtr BaseState::getObservable() const {
    return ParameterDag::GetBase(mKey);
}

void BaseState::set(bool state) const { getObservable()->set(state); }

// NodeValue
NodeValue::NodeValue(WizardId id, param_t key) : ValueParam(id, key, false) {}

NodeValueObservablePtr NodeValue::getObservable() const {
    return ParameterDag::GetNode(mId, mKey);
}

ParameterSubscriptionPtr NodeValue::subscribeTo(
    const std::initializer_list<ValueParam>& values,
    const std::initializer_list<StateParam>& states,
    std::function<Number()> func) const {
    auto id = mId;
    auto key = mKey;
    ParameterDag::GetNode(id, key)->set(func());
    return ParameterSystem::subscribe(values, states, [id, key, func]() {
        ParameterDag::GetNode(id, key)->set(func());
    });
}

ParameterSubscriptionPtr NodeValue::subscribeTo(
    ValueParam param, std::function<Number(const Number&)> func) const {
    auto id = mId;
    auto key = mKey;
    ParameterDag::GetNode(id, key)->set(func(param.get()));
    return ParameterDag::GetNode(mId, mKey)->subscribe(
        [id, key, param, func]() {
            ParameterDag::GetNode(id, key)->set(func(param.get()));
        });
}

ParameterSubscriptionPtr NodeValue::subscribeTo(
    StateParam param, std::function<Number(bool)> func) const {
    auto id = mId;
    auto key = mKey;
    ParameterDag::GetNode(id, key)->set(func(param.get()));
    return ParameterDag::GetNode(mId, mKey)->subscribe(
        [id, key, param, func]() {
            ParameterDag::GetNode(id, key)->set(func(param.get()));
        });
}

// NodeState
NodeState::NodeState(param_t key) : StateParam(key, false) {}

NodeStateObservablePtr NodeState::getObservable() const {
    return ParameterDag::GetNode(mKey);
}

ParameterSubscriptionPtr NodeState::subscribeTo(
    const std::initializer_list<ValueParam>& values,
    const std::initializer_list<StateParam>& states,
    std::function<bool()> func) const {
    auto key = mKey;
    ParameterDag::GetNode(key)->set(func());
    return ParameterSystem::subscribe(values, states, [key, func]() {
        ParameterDag::GetNode(key)->set(func());
    });
}

ParameterSubscriptionPtr NodeState::subscribeTo(
    ValueParam param, std::function<bool(const Number&)> func) const {
    auto key = mKey;
    ParameterDag::GetNode(key)->set(func(param.get()));
    return ParameterDag::GetNode(mKey)->subscribe([key, param, func]() {
        ParameterDag::GetNode(key)->set(func(param.get()));
    });
}

ParameterSubscriptionPtr NodeState::subscribeTo(
    StateParam param, std::function<bool(bool)> func) const {
    auto key = mKey;
    ParameterDag::GetNode(key)->set(func(param.get()));
    return ParameterDag::GetNode(mKey)->subscribe([key, param, func]() {
        ParameterDag::GetNode(key)->set(func(param.get()));
    });
}

// Param creators
BaseState Param(State::B key) { return BaseState(key); }

NodeState Param(State::N key) { return NodeState(key); }

// Subscribing
ParameterSubscriptionPtr subscribe(
    const std::initializer_list<ValueParam>& values,
    const std::initializer_list<StateParam>& states,
    std::function<void()> func) {
    ParameterSubscriptionPtr sub;
    for (auto val : values) {
        if (!sub) {
            sub = val.getObservable()->subscribe(func);
        } else {
            val.getObservable()->subscribe(func);
        }
    }
    for (auto state : values) {
        if (!sub) {
            sub = state.getObservable()->subscribe(func);
        } else {
            state.getObservable()->subscribe(func);
        }
    }
    return sub;
}

// States
BaseState States::operator[](State::B key) { return Param(key); }

NodeState States::operator[](State::N key) { return Param(key); }

bool SetDefault(State::B key, bool val) {
    Param(key).getObservable()->mDefault = val;
    return true;
}
}  // namespace ParameterSystem
