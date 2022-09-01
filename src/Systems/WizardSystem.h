#ifndef WIZARD_SYSTEM_H
#define WIZARD_SYSTEM_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/TargetSystem.h>
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

// For handling wizard animations/images
typedef TargetSystem::TargetDataObservable<WizardId, RenderData>
    WizardImageObservable;

const RenderData& GetWizardImage(WizardId id);

std::shared_ptr<WizardImageObservable> GetWizardImageObservable();

// For handling wizard positions
typedef TargetSystem::TargetDataObservable<WizardId, SDL_FPoint>
    WizardPosObservable;

SDL_FPoint GetWizardPos(WizardId id);

std::shared_ptr<WizardPosObservable> GetWizardPosObservable();

enum Event : uint8_t {
    NoReset = 0,
    ResetT1,
    ResetT2,

    TimeWarp,
};

typedef TargetSystem::TargetObservable<Event> WizardEventObservableBase;
class WizardEventObservable : public WizardEventObservableBase {
   public:
    void next(Event e);
};

std::shared_ptr<WizardEventObservable> GetWizardEventObservable();

class WizardService
    : public Service<HideObservable, WizardEventObservable,
                     WizardImageObservable, WizardPosObservable> {};
}  // namespace WizardSystem

#endif
