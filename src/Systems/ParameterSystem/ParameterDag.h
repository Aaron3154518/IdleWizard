#ifndef PARAMETER_DAG_H
#define PARAMETER_DAG_H

#include <Systems/ParameterSystem/ParameterObservable.h>
#include <Systems/ParameterSystem/WizardParameters.h>
#include <Systems/ParameterSystem/WizardStates.h>
#include <Wizards/WizardIds.h>

#include <unordered_map>

namespace ParameterSystem {
typedef std::unordered_map<WizardId,
                           std::unordered_map<param_t, BaseValueObservablePtr>>
    BaseValueMap;
typedef std::unordered_map<WizardId,
                           std::unordered_map<param_t, NodeValueObservablePtr>>
    NodeValueMap;
typedef std::unordered_map<param_t, BaseStateObservablePtr> BaseStateMap;
typedef std::unordered_map<param_t, NodeStateObservablePtr> NodeStateMap;

class ParameterDagImpl {
   protected:
    static const BaseValueObservablePtr& GetBase(WizardId id, param_t key);
    static const NodeValueObservablePtr& GetNode(WizardId id, param_t key);
    static const BaseStateObservablePtr& GetBase(param_t key);
    static const NodeStateObservablePtr& GetNode(param_t key);

   private:
    static BaseValueMap& mBaseValues();
    static NodeValueMap& mNodeValues();
    static BaseStateMap& mBaseStates();
    static NodeStateMap& mNodeStates();
};

class ParameterDag : public ParameterDagImpl {
    template <WizardId>
    friend class Params;

    friend class ValueParam;
    friend class BaseValue;
    friend class NodeValue;
    friend class StateParam;
    friend class BaseState;
    friend class NodeState;
};
}  // namespace ParameterSystem

#endif
