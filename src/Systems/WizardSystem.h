#ifndef WIZARD_SYSTEM_H
#define WIZARD_SYSTEM_H

#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Wizards/WizardIds.h>

namespace WizardSystem {
// For handling wizard hide/show events
typedef ForwardObservable<void(WizardId, bool)> HideObservableBase;
class HideObservable : public HideObservableBase {
   public:
    void next(WizardId id, bool hide);

    bool isHidden(WizardId id) const;

   private:
    std::unordered_map<WizardId, bool> mHidden;
};

bool Hidden(WizardId id);

std::shared_ptr<HideObservable> GetHideObservable();

enum ResetTier : uint8_t {
    T1 = 0,
    T2,

    NONE,
};

typedef ForwardObservable<void(ResetTier)> ResetObservableBase;
class ResetObservable : public ResetObservableBase {};

void Reset(ResetTier tier);

std::shared_ptr<ResetObservable> GetResetObservable();

class WizardService : public Service<HideObservable, ResetObservable> {};
}  // namespace WizardSystem

#endif
