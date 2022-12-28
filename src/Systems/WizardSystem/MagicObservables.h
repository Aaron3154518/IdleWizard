#ifndef MAGIC_OBSERVABLES_H
#define MAGIC_OBSERVABLES_H

#include <Wizards/Wizard/WizardFireball.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>

#include <memory>

namespace WizardSystem {
typedef ForwardObservable<void(const Number&)> MagicObservableBase;

class CatalystMagicObservable : public MagicObservableBase {
   public:
    using MagicObservableBase::next;

    void next(const Wizard::Fireball& fb);
};

std::shared_ptr<CatalystMagicObservable> GetCatalystMagicObservable();

class MagicService : public Service<CatalystMagicObservable> {};
}  // namespace WizardSystem

#endif
