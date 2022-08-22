#include "Parameter.h"

namespace ParameterSystem {
// ParamBase
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

// ParamListEquals
bool ParamMapBase::ParamListEquals::operator()(
    const ParamMapBase::ParamList& lhs,
    const ParamMapBase::ParamList& rhs) const {
    return lhs.mId == rhs.mId;
}

// ParamListHash
size_t ParamMapBase::ParamListHash::operator()(
    const ParamMapBase::ParamList& params) const {
    return std::hash<WizardId>()(params.mId);
}

// ParamMapBase
ParamMapBase::ParamMapBase() {}
ParamMapBase::ParamMapBase(const ParamListSet& keys) : mKeys(keys) {}
ParamMapBase::ParamMapBase(const ParamBase& param)
    : mKeys({ParamList{param.mId, {param.mKey}}}) {}

ParameterObservable::SubscriptionPtr ParamMapBase::subscribe(
    std::function<void()> func) const {
    auto params = ParameterSystem::Get();
    ParameterObservable::SubscriptionPtr sub;
    for (auto& list : mKeys) {
        for (auto key : list.mKeys) {
            if (!sub) {
                sub = params->get(list.mId, key)->subscribe(func);
            } else {
                params->get(list.mId, key)->subscribe(sub);
            }
        }
    }
    return sub;
}

}  // namespace ParameterSystem
