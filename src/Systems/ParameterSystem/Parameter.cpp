#include "Parameter.h"

namespace ParameterSystem {
// ParamBase
ParamBase::ParamBase() {}
ParamBase::ParamBase(WizardId id, param_t key) : mId(id), mKey(key) {}

const Number& ParamBase::get() const {
    return ParameterSystem::Get()->get(mId, mKey)->get();
}

void ParamBase::set(const Number& val) const {
    ParameterSystem::Get()->get(mId, mKey)->set(val);
}

ParameterObservable::SubscriptionPtr ParamBase::subscribe(
    std::function<void()> func) const {
    return ParameterSystem::Get()->get(mId, mKey)->subscribe(func);
}

// ParamListBase
ParamListBase::ParamListBase() {}
ParamListBase::ParamListBase(WizardId id,
                             const std::unordered_set<param_t>& keys)
    : mId(id), mKeys(keys) {}

ParamListBase::ParamListBase(const ParamBase& param)
    : mId(param.mId), mKeys({param.mKey}) {}

const Number& ParamListBase::get(param_t key) const {
    auto it = mKeys.find(key);
    if (it == mKeys.end()) {
        throw std::runtime_error(
            "ParamList::get(): Key " + std::to_string(key) +
            " not in parameter list for " + WIZ_NAMES.at(mId));
    }
    return ParameterSystem::Get()->get(mId, key)->get();
}

void ParamListBase::set(param_t key, const Number& val) const {
    auto it = mKeys.find(key);
    if (it == mKeys.end()) {
        throw std::runtime_error(
            "ParamList::set(): Key " + std::to_string(key) +
            " not in parameter list for " + WIZ_NAMES.at(mId));
    }
    return ParameterSystem::Get()->get(mId, key)->set(val);
}

ParameterObservable::SubscriptionPtr ParamListBase::subscribe(
    std::function<void()> func) const {
    ParameterObservable::SubscriptionPtr sub;
    for (auto key : mKeys) {
        if (!sub) {
            sub = ParameterSystem::Get()->get(mId, key)->subscribe(func);
        } else {
            ParameterSystem::Get()->get(mId, key)->subscribe(sub);
        }
    }
    return sub;
}

void ParamListBase::subscribe(ParameterObservable::SubscriptionPtr sub) const {
    for (auto key : mKeys) {
        ParameterSystem::Get()->get(mId, key)->subscribe(sub);
    }
}

// ParamMapBase
ParamMapBase::ParamMapBase() {}
ParamMapBase::ParamMapBase(const ParamListSet& keys) : mKeys(keys) {}
ParamMapBase::ParamMapBase(const ParamBase& param)
    : mKeys({ParamListBase(param)}) {}
ParamMapBase::ParamMapBase(const ParamListBase& params) : mKeys({params}) {}

const Number& ParamMapBase::_get(const ParamListBase& params,
                                 param_t key) const {
    auto it = mKeys.find(params);
    if (it == mKeys.end()) {
        throw std::runtime_error(
            "ParamMap::get(): " + WIZ_NAMES.at(params.mId) +
            " not in parameter map");
    }
    return it->get(key);
}

void ParamMapBase::_set(const ParamListBase& params, param_t key,
                        const Number& val) const {
    auto it = mKeys.find(params);
    if (it == mKeys.end()) {
        throw std::runtime_error(
            "ParamMap::get(): " + WIZ_NAMES.at(params.mId) +
            " not in parameter map");
    }
    it->set(key, val);
}

ParameterObservable::SubscriptionPtr ParamMapBase::subscribe(
    std::function<void()> func) const {
    ParameterObservable::SubscriptionPtr sub;
    for (auto& params : mKeys) {
        if (!sub) {
            sub = params.subscribe(func);
        } else {
            params.subscribe(sub);
        }
    }
    return sub;
}

}  // namespace ParameterSystem
