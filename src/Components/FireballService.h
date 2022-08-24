#ifndef FIREBALL_SERVICE_H
#define FIREBALL_SERVICE_H

#include <SDL.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <Systems/TargetSystem.h>
#include <Utils/Number.h>
#include <Wizards/WizardIds.h>

// Forward declaration
class Fireball;

typedef Observable<void(SDL_FPoint), WizardId> FireballObservableBase;

class FireballObservable : public FireballObservableBase {
   public:
    enum : size_t { FUNC = 0, DATA };

    void next(WizardId id, SDL_FPoint pos);

    SDL_FPoint getPos(WizardId id) const;

   private:
    void onSubscribe(SubscriptionPtr sub);

    SDL_FPoint mTargets[WizardId::size];
};

// Rename me
typedef TargetSystem::TargetObservable<const Fireball&> FireballHitObservable;
typedef TargetSystem::TargetObservable<Fireball&, const Number&>
    FireballFireRingHitObservable;

class FireballService
    : public Service<FireballObservable, FireballHitObservable,
                     FireballFireRingHitObservable> {};

#endif
