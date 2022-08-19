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
template <WizardId id>
using WizardTypeList = std::initializer_list<WizardType<id>>;

template <WizardId id>
std::unordered_set<param_t> toSet(WizardTypeList<id> list) {
    std::unordered_set<param_t> result;
    for (auto key : list) {
        result.insert(static_cast<param_t>(key));
    }
    return result;
}

// Base class for different Param templates
struct ParamBase {
    friend class ParamListBase;
    friend class ParamMapBase;

   public:
    ParamBase();
    virtual ~ParamBase() = default;

    const Number& get() const;

    void set(const Number& val) const;

    ParameterObservable::SubscriptionPtr subscribe(
        std::function<void()> func) const;

   protected:
    ParamBase(WizardId id, param_t key);

    WizardId mId;
    param_t mKey;
};

typedef std::shared_ptr<ParamBase> ParamBasePtr;
typedef std::unique_ptr<ParamBase> ParamBaseUPtr;

// Stores one WizardId and WizardType
template <WizardId id>
struct Param : public ParamBase {
   public:
    Param(WizardType<id> key) : ParamBase(id, key) {}

    static ParamBasePtr New(WizardType<id> key) {
        return std::make_shared<Param<id>>(key);
    }
};

// Base class for different ParamList templates
struct ParamListBase {
    friend class ParamListEquals;
    friend class ParamListHash;
    friend class ParamMapBase;

   public:
    ParamListBase();
    ParamListBase(const ParamBase& param);
    virtual ~ParamListBase() = default;

    const Number& get(param_t key) const;

    void set(param_t key, const Number& val) const;

    ParameterObservable::SubscriptionPtr subscribe(
        std::function<void()> func) const;

   protected:
    ParamListBase(WizardId id, const std::unordered_set<param_t>& keys);

    WizardId mId;
    std::unordered_set<param_t> mKeys;

   private:
    void subscribe(ParameterObservable::SubscriptionPtr sub) const;
};

typedef std::shared_ptr<ParamListBase> ParamListBasePtr;
typedef std::unique_ptr<ParamListBase> ParamListBaseUPtr;

// Stores one WizardId and multiple keys
template <WizardId id>
struct ParamList : public ParamListBase {
    ParamList() : ParamListBase(id, {}) {}
    ParamList(const WizardTypeList<id>& keys)
        : ParamListBase(id, toSet<id>(keys)) {}

    static ParamListBasePtr New(const WizardTypeList<id>& keys) {
        return std::make_shared<ParamList<id>>(keys);
    }

    static const Number& Get(WizardType<id> key) {
        return ParameterSystem::Get()->get(id, key)->get();
    }

    static void Set(WizardType<id> key, const Number& val) {
        ParameterSystem::Get()->get(id, key)->set(val);
    }
};

struct ParamListEquals {
   public:
    bool operator()(const ParamListBase& lhs, const ParamListBase& rhs) const {
        return lhs.mId == rhs.mId;
    }
};

struct ParamListHash {
   public:
    size_t operator()(const ParamListBase& params) const {
        return std::hash<WizardId>()(params.mId);
    }
};

typedef std::unordered_set<ParamListBase, ParamListHash, ParamListEquals>
    ParamListSet;

// Base class for different ParamMap templates
struct ParamMapBase {
   public:
    ParamMapBase();
    ParamMapBase(const ParamBase& param);
    ParamMapBase(const ParamListBase& params);
    ~ParamMapBase() = default;

    template <WizardId id>
    const Number& get(WizardType<id> key) const {
        return _get(ParamList<id>(), key);
    }

    template <WizardId id>
    void set(WizardType<id> key, const Number& val) const {
        _set(ParamList<id>(), key, val);
    }

    ParameterObservable::SubscriptionPtr subscribe(
        std::function<void()> func) const;

   protected:
    ParamMapBase(const ParamListSet& keys);

    ParamListSet mKeys;

   private:
    const Number& _get(const ParamListBase& params, param_t key) const;

    void _set(const ParamListBase& params, param_t key,
              const Number& val) const;
};

typedef std::shared_ptr<ParamListBase> ParamListBasePtr;
typedef std::unique_ptr<ParamMapBase> ParamMapBaseUPtr;

// Stores multiple WizardIds and WizardTypes
template <WizardId... ids>
struct ParamMap : public ParamMapBase {
    ParamMap() : ParamMapBase({ParamList<ids>()...}) {}
    ParamMap(const WizardTypeList<ids>&... keys)
        : ParamMapBase({ParamList<ids>(keys)...}) {}

    static ParamListBasePtr New(const WizardTypeList<ids>&... keys) {
        return std::make_shared<ParamMap<ids...>>(keys...);
    }
};
}  // namespace ParameterSystem

#endif
