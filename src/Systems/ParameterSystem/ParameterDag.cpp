#include "ParameterDag.h"

namespace ParameterSystem {
// ParameterDagImpl
const BaseValueObservablePtr& ParameterDagImpl::GetBase(WizardId id,
                                                        param_t key) {
    auto& map = mBaseValues();
    auto& ptr = map[id][key];
    if (!ptr) {
        ptr = std::make_shared<BaseValueObservable>();
    }
    return ptr;
}

const NodeValueObservablePtr& ParameterDagImpl::GetNode(WizardId id,
                                                        param_t key) {
    auto& map = mNodeValues();
    auto& ptr = map[id][key];
    if (!ptr) {
        ptr = std::make_shared<NodeValueObservable>();
    }
    return ptr;
}

const BaseStateObservablePtr& ParameterDagImpl::GetBase(param_t key) {
    auto& map = mBaseStates();
    auto& ptr = map[key];
    if (!ptr) {
        ptr = std::make_shared<BaseStateObservable>();
    }
    return ptr;
}

const NodeStateObservablePtr& ParameterDagImpl::GetNode(param_t key) {
    auto& map = mNodeStates();
    auto& ptr = map[key];
    if (!ptr) {
        ptr = std::make_shared<NodeStateObservable>();
    }
    return ptr;
}

BaseValueMap& ParameterDagImpl::mBaseValues() {
    static BaseValueMap map;
    return map;
}
NodeValueMap& ParameterDagImpl::mNodeValues() {
    static NodeValueMap map;
    return map;
}
BaseStateMap& ParameterDagImpl::mBaseStates() {
    static BaseStateMap map;
    return map;
}
NodeStateMap& ParameterDagImpl::mNodeStates() {
    static NodeStateMap map;
    return map;
}
}  // namespace ParameterSystem
