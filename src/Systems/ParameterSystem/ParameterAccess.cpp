#include "ParameterAccess.h"

namespace ParameterSystem {
// ParamImpl
ParamImpl::ParamImpl(WizardId id, key_t key, bool isBase)
    : mId(id), mKey(key), mIsBase(isBase) {}
ParamImpl::ParamImpl(const ParamImpl& other)
    : ParamImpl(other.mId, other.mKey, other.mIsBase) {}

ParamImpl& ParamImpl::operator=(const ParamImpl& other) {
    mId = other.mId;
    mKey = other.mKey;
    mIsBase = other.mIsBase;
    return *this;
}
bool ParamImpl::operator==(const ParamImpl& other) const {
    return mId == other.mId && mKey == other.mKey;
}

WizardId ParamImpl::id() const { return mId; }
key_t ParamImpl::key() const { return mKey; }

// ValueParam
ValueObservablePtr ValueParam::operator->() const {
    if (mIsBase) {
        return ParameterDag::GetBaseValue(mId, mKey);
    }
    return ParameterDag::GetNodeValue(mId, mKey);
}

const Number& ValueParam::get() const { return operator->()->get(); }

ParameterSubscriptionPtr ValueParam::subscribe(std::function<void()> func,
                                               bool fire) const {
    if (fire) {
        func();
    }
    return operator->()->subscribe(func);
}
ParameterSubscriptionPtr ValueParam::subscribe(
    std::function<void(const Number&)> func, bool fire) const {
    auto id = mId;
    auto key = mKey;
    if (mIsBase) {
        return subscribe(
            [id, key, func]() {
                func(ParameterDag::GetBaseValue(id, key)->get());
            },
            fire);
    }
    return subscribe(
        [id, key, func]() { func(ParameterDag::GetNodeValue(id, key)->get()); },
        fire);
}

// StateParam
StateObservablePtr StateParam::operator->() const {
    if (mIsBase) {
        return ParameterDag::GetBaseState(mId, mKey);
    }
    return ParameterDag::GetNodeState(mId, mKey);
}

bool StateParam::get() const { return operator->()->get(); }

ParameterSubscriptionPtr StateParam::subscribe(std::function<void()> func,
                                               bool fire) const {
    if (fire) {
        func();
    }
    return operator->()->subscribe(func);
}
ParameterSubscriptionPtr StateParam::subscribe(std::function<void(bool)> func,
                                               bool fire) const {
    auto id = mId;
    auto key = mKey;
    if (mIsBase) {
        return subscribe(
            [id, key, func]() {
                func(ParameterDag::GetBaseState(id, key)->get());
            },
            fire);
    }
    return subscribe(
        [id, key, func]() { func(ParameterDag::GetNodeState(id, key)->get()); },
        fire);
}

// BaseValue
BaseValue::BaseValue(WizardId id, key_t key) : ValueParam(id, key, true) {}

BaseValueObservablePtr BaseValue::operator->() const {
    return ParameterDag::GetBaseValue(mId, mKey);
}

void BaseValue::set(const Number& val) const { operator->()->set(val); }

// BaseState
BaseState::BaseState(WizardId id, key_t key) : StateParam(id, key, true) {}

BaseStateObservablePtr BaseState::operator->() const {
    return ParameterDag::GetBaseState(mId, mKey);
}

void BaseState::set(bool state) const { operator->()->set(state); }

// NodeValue
NodeValue::NodeValue(WizardId id, key_t key) : ValueParam(id, key, false) {}

NodeValueObservablePtr NodeValue::operator->() const {
    return ParameterDag::GetNodeValue(mId, mKey);
}

ParameterSubscriptionPtr NodeValue::subscribeTo(
    const std::initializer_list<ValueParam>& values,
    const std::initializer_list<StateParam>& states,
    std::function<Number()> func, bool fire) const {
    auto id = mId;
    auto key = mKey;
    return ParameterSystem::subscribe(
        values, states,
        [id, key, func]() { ParameterDag::GetNodeValue(id, key)->set(func()); },
        fire);
}

ParameterSubscriptionPtr NodeValue::subscribeTo(
    ValueParam param, std::function<Number(const Number&)> func,
    bool fire) const {
    auto id = mId;
    auto key = mKey;
    return param.subscribe(
        [id, key, func](const Number& val) {
            ParameterDag::GetNodeValue(id, key)->set(func(val));
        },
        fire);
}

ParameterSubscriptionPtr NodeValue::subscribeTo(
    StateParam param, std::function<Number(bool)> func, bool fire) const {
    auto id = mId;
    auto key = mKey;
    return param.subscribe(
        [id, key, func](bool state) {
            ParameterDag::GetNodeValue(id, key)->set(func(state));
        },
        fire);
}

// NodeState
NodeState::NodeState(WizardId id, key_t key) : StateParam(id, key, false) {}

NodeStateObservablePtr NodeState::operator->() const {
    return ParameterDag::GetNodeState(mId, mKey);
}

ParameterSubscriptionPtr NodeState::subscribeTo(
    const std::initializer_list<ValueParam>& values,
    const std::initializer_list<StateParam>& states, std::function<bool()> func,
    bool fire) const {
    auto id = mId;
    auto key = mKey;
    return ParameterSystem::subscribe(
        values, states,
        [id, key, func]() { ParameterDag::GetNodeState(id, key)->set(func()); },
        fire);
}

ParameterSubscriptionPtr NodeState::subscribeTo(
    ValueParam param, std::function<bool(const Number&)> func,
    bool fire) const {
    auto id = mId;
    auto key = mKey;
    return param.subscribe(
        [id, key, func](const Number& val) {
            ParameterDag::GetNodeState(id, key)->set(func(val));
        },
        fire);
}

ParameterSubscriptionPtr NodeState::subscribeTo(StateParam param,
                                                std::function<bool(bool)> func,
                                                bool fire) const {
    auto id = mId;
    auto key = mKey;
    return param.subscribe(
        [id, key, func](bool state) {
            ParameterDag::GetNodeState(id, key)->set(func(state));
        },
        fire);
}

// Subscribing
ParameterSubscriptionPtr subscribe(
    const std::initializer_list<ValueParam>& values,
    const std::initializer_list<StateParam>& states, std::function<void()> func,
    bool fire) {
    ParameterSubscriptionPtr sub;
    for (auto val : values) {
        if (!sub) {
            sub = val->subscribe(func);
        } else {
            val->subscribe(sub);
        }
    }
    for (auto state : states) {
        if (!sub) {
            sub = state->subscribe(func);
        } else {
            state->subscribe(sub);
        }
    }
    if (fire) {
        func();
    }
    return sub;
}
}  // namespace ParameterSystem
