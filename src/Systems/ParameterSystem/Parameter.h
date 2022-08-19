#ifndef PARAMETER_H
#define PARAMETER_H

#include <Systems/ParameterSystem/ParameterObservable.h>
#include <Systems/ParameterSystem/ParameterService.h>
#include <Systems/ParameterSystem/WizardParams.h>
#include <Utils/Number.h>
#include <Wizards/WizardIds.h>

#include <exception>
#include <functional>
#include <initializer_list>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ParameterSystem {
// Provides easy access to params from one wizard
template <WizardId id>
struct Params {
    static const Number& get(WizardType<id> key) {
        return ParameterSystem::Get()->get(id, key)->get();
    }

    static void set(WizardType<id> key, const Number& val) {
        ParameterSystem::Get()->get(id, key)->set(val);
    }
};

// Base class for different Param templates
struct ParamBase {
   public:
    virtual ~ParamBase() = default;

    const Number& get() const;

    void set(const Number& val) const;

    ParameterObservable::SubscriptionPtr subscribe(
        std::function<void()> func) const;

    const WizardId mId;
    const param_t mKey;

   protected:
    ParamBase(WizardId id, param_t key);
};

typedef std::shared_ptr<ParamBase> ParamBasePtr;
typedef std::unique_ptr<ParamBase> ParamBaseUPtr;

// Stores one WizardId and WizardType
template <WizardId id>
struct Param : public ParamBase {
   public:
    Param(WizardType<id> key) : ParamBase(id, key) {}
};

// Base class for different ParamMap templates
struct ParamMapBase {
   private:
    // Stores one WizardId and multiple keys
    struct ParamList {
        const WizardId mId;
        const std::unordered_set<param_t> mKeys;
    };

    struct ParamListEquals {
        bool operator()(const ParamList& lhs, const ParamList& rhs) const;
    };

    struct ParamListHash {
        size_t operator()(const ParamList& params) const;
    };

    typedef std::unordered_set<ParamList, ParamListHash, ParamListEquals>
        ParamListSet;

   public:
    ParamMapBase();
    ParamMapBase(const ParamBase& param);
    ~ParamMapBase() = default;

    ParameterObservable::SubscriptionPtr subscribe(
        std::function<void()> func) const;

    const ParamListSet mKeys;

   protected:
    ParamMapBase(const ParamListSet& keys);
};

template <WizardId id>
using WizardTypeList = std::initializer_list<WizardType<id>>;

template <WizardId id>
std::unordered_set<param_t> toSet(const WizardTypeList<id>& list) {
    std::unordered_set<param_t> result;
    for (auto key : list) {
        result.insert(static_cast<param_t>(key));
    }
    return result;
}

// Stores multiple WizardIds and WizardTypes
template <WizardId... ids>
struct ParamMap : public ParamMapBase {
    ParamMap(const WizardTypeList<ids>&... keys)
        : ParamMapBase({ParamList{ids, toSet<ids>(keys)}...}) {}
};
}  // namespace ParameterSystem

#endif
