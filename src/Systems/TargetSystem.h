#ifndef TARGET_SYSTEM_H
#define TARGET_SYSTEM_H

#include <ServiceSystem/Observable.h>
#include <Wizards/WizardIds.h>

namespace TargetSystem {
// Observable for triggering events for a specific wizard
template <class T, class... Ts>
using TargetObservableBase = Observable<void(T, Ts...), WizardId>;
template <class T, class... Ts>
class TargetObservable : public TargetObservableBase<T, Ts...> {
   public:
    enum : size_t { FUNC = 0, DATA };

    void next(WizardId target, T t, Ts... args) {
        for (auto sub : *this) {
            if (sub->template get<DATA>() == target) {
                sub->template get<FUNC>()(t, args...);
            }
        }
    }
};
}  // namespace TargetSystem

#endif
