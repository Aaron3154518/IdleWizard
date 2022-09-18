#ifndef MAGIC_OBSERVABLES_H
#define MAGIC_OBSERVABLES_H

#include <Components/Fireballs/WizardFireball.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>

#include <memory>

namespace WizardSystem {
typedef ForwardObservable<void(const Number&), void(const WizardFireball&)>
    MagicObservableBase;

class CrystalMagicObservable : public MagicObservableBase {};

std::shared_ptr<CrystalMagicObservable> GetCrystalMagicObservable();

class CatalystMagicObservable : public MagicObservableBase {};

std::shared_ptr<CatalystMagicObservable> GetCatalystMagicObservable();

class MagicService
    : public Service<CrystalMagicObservable, CatalystMagicObservable> {};
}  // namespace WizardSystem

#endif
