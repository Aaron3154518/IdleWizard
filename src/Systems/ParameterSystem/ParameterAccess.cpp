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

ParameterSubscriptionPtr ValueParam::subscribe(std::function<void()> func,
                                               bool fire) const {
    if (fire) {
        func();
    }
    return getObservable()->subscribe(func);
}
ParameterSubscriptionPtr ValueParam::subscribe(
    std::function<void(const Number&)> func, bool fire) const {
    auto id = mId;
    auto key = mKey;
    if (mIsBase) {
        return subscribe(
            [id, key, func]() { func(ParameterDag::GetBase(id, key)->get()); },
            fire);
    }
    return subscribe(
        [id, key, func]() { func(ParameterDag::GetNode(id, key)->get()); },
        fire);
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

ParameterSubscriptionPtr StateParam::subscribe(std::function<void()> func,
                                               bool fire) const {
    if (fire) {
        func();
    }
    return getObservable()->subscribe(func);
}
ParameterSubscriptionPtr StateParam::subscribe(std::function<void(bool)> func,
                                               bool fire) const {
    auto key = mKey;
    if (mIsBase) {
        return subscribe(
            [key, func]() { func(ParameterDag::GetBase(key)->get()); }, fire);
    }
    return subscribe([key, func]() { func(ParameterDag::GetNode(key)->get()); },
                     fire);
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
    std::function<Number()> func, bool fire) const {
    auto id = mId;
    auto key = mKey;
    return ParameterSystem::subscribe(
        values, states,
        [id, key, func]() { ParameterDag::GetNode(id, key)->set(func()); },
        fire);
}

ParameterSubscriptionPtr NodeValue::subscribeTo(
    ValueParam param, std::function<Number(const Number&)> func,
    bool fire) const {
    auto id = mId;
    auto key = mKey;
    return param.subscribe(
        [id, key, func](const Number& val) {
            ParameterDag::GetNode(id, key)->set(func(val));
        },
        fire);
}

ParameterSubscriptionPtr NodeValue::subscribeTo(
    StateParam param, std::function<Number(bool)> func, bool fire) const {
    auto id = mId;
    auto key = mKey;
    return param.subscribe(
        [id, key, func](bool state) {
            ParameterDag::GetNode(id, key)->set(func(state));
        },
        fire);
}

// NodeState
NodeState::NodeState(param_t key) : StateParam(key, false) {}

NodeStateObservablePtr NodeState::getObservable() const {
    return ParameterDag::GetNode(mKey);
}

ParameterSubscriptionPtr NodeState::subscribeTo(
    const std::initializer_list<ValueParam>& values,
    const std::initializer_list<StateParam>& states, std::function<bool()> func,
    bool fire) const {
    auto key = mKey;
    return ParameterSystem::subscribe(
        values, states,
        [key, func]() { ParameterDag::GetNode(key)->set(func()); }, fire);
}

ParameterSubscriptionPtr NodeState::subscribeTo(
    ValueParam param, std::function<bool(const Number&)> func,
    bool fire) const {
    auto key = mKey;
    return param.subscribe(
        [key, func](const Number& val) {
            ParameterDag::GetNode(key)->set(func(val));
        },
        fire);
}

ParameterSubscriptionPtr NodeState::subscribeTo(StateParam param,
                                                std::function<bool(bool)> func,
                                                bool fire) const {
    auto key = mKey;
    return param.subscribe(
        [key, func](bool state) {
            ParameterDag::GetNode(key)->set(func(state));
        },
        fire);
}

// Param creators
BaseState Param(State::B key) { return BaseState(key); }

NodeState Param(State::N key) { return NodeState(key); }

// Subscribing
ParameterSubscriptionPtr subscribe(
    const std::initializer_list<ValueParam>& values,
    const std::initializer_list<StateParam>& states, std::function<void()> func,
    bool fire) {
    ParameterSubscriptionPtr sub;
    for (auto val : values) {
        if (!sub) {
            sub = val.getObservable()->subscribe(func);
        } else {
            val.getObservable()->subscribe(sub);
        }
    }
    for (auto state : values) {
        if (!sub) {
            sub = state.getObservable()->subscribe(func);
        } else {
            state.getObservable()->subscribe(sub);
        }
    }
    if (fire) {
        func();
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
