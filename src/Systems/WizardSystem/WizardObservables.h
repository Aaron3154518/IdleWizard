#ifndef WIZARD_OBSERVABLES_H
#define WIZARD_OBSERVABLES_H

#include <RenderSystem/RenderTypes.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/TargetSystem.h>
#include <Wizards/WizardIds.h>

namespace WizardSystem {
// For handling wizard hide/show events
typedef TargetSystem::TargetDataObservable<WizardId, bool> HideObservable;

bool Hidden(WizardId id);

std::shared_ptr<HideObservable> GetHideObservable();

// For handling wizard animations/images
typedef TargetSystem::TargetDataObservable<WizardId, RenderData>
    WizardImageObservable;

const RenderData& GetWizardImage(WizardId id);

std::shared_ptr<WizardImageObservable> GetWizardImageObservable();

// For handling wizard positions
typedef TargetSystem::TargetDataObservable<WizardId, Rect> WizardPosObservable;

const Rect& GetWizardPos(WizardId id);

std::shared_ptr<WizardPosObservable> GetWizardPosObservable();

class WizardDataService : public Service<HideObservable, WizardImageObservable,
                                         WizardPosObservable> {};
}  // namespace WizardSystem

#endif
