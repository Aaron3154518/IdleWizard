#include "ParameterDag.h"

namespace ParameterSystem {
// ParameterDagImpl
const BaseValueObservablePtr& ParameterDagImpl::GetBase(WizardId id,
                                                        param_t key) {
    auto& map = BaseValueMap();
    auto& ptr = map[id][key];
    if (!ptr) {
        ptr = std::make_shared<BaseValueObservable>();
    }
    return ptr;
}

const NodeValueObservablePtr& ParameterDagImpl::GetNode(WizardId id,
                                                        param_t key) {
    auto& map = NodeValueMap();
    auto& ptr = map[id][key];
    if (!ptr) {
        ptr = std::make_shared<NodeValueObservable>();
    }
    return ptr;
}

const BaseStateObservablePtr& ParameterDagImpl::GetBase(param_t key) {
    auto& map = BaseStateMap();
    auto& ptr = map[key];
    if (!ptr) {
        ptr = std::make_shared<BaseStateObservablePtr>();
    }
    return ptr;
}

const NodeStateObservablePtr& ParameterDagImpl::GetNode(param_t key) {
    auto& map = NodeStateMap();
    auto& ptr = map[key];
    if (!ptr) {
        ptr = std::make_shared<NodeStateObservablePtr>();
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
