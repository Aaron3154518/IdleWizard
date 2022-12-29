#ifndef PARAMETER_DAG_H
#define PARAMETER_DAG_H

#include <Systems/ParameterSystem/ParameterObservable.h>
#include <Wizards/WizardIds.h>

#include <unordered_map>

namespace ParameterSystem {
typedef uint8_t key_t;

typedef std::unordered_map<WizardId,
                           std::unordered_map<key_t, BaseValueObservablePtr>>
    BaseValueMap;
typedef std::unordered_map<WizardId,
                           std::unordered_map<key_t, NodeValueObservablePtr>>
    NodeValueMap;
typedef std::unordered_map<key_t, BaseStateObservablePtr> BaseStateMap;
typedef std::unordered_map<key_t, NodeStateObservablePtr> NodeStateMap;

class ParameterDagImpl {
   protected:
    static const BaseValueObservablePtr& GetBase(WizardId id, key_t key);
    static const NodeValueObservablePtr& GetNode(WizardId id, key_t key);
    static const BaseStateObservablePtr& GetBase(key_t key);
    static const NodeStateObservablePtr& GetNode(key_t key);

   private:
    static BaseValueMap& mBaseValues();
    static NodeValueMap& mNodeValues();
    static BaseStateMap& mBaseStates();
    static NodeStateMap& mNodeStates();
};

class ParameterDag : public ParameterDagImpl {
    friend class ValueParam;
    friend class BaseValue;
    friend class NodeValue;
    friend class StateParam;
    friend class BaseState;
    friend class NodeState;
};
}  // namespace ParameterSystem

#endif
