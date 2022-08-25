#ifndef FIREBALL_TARGET_OBSERVABLE_H
#define FIREBALL_TARGET_OBSERVABLE_H

#include <SDL.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <Wizards/WizardIds.h>

typedef Observable<void(SDL_FPoint), WizardId> FireballTargetPosObservableBase;

class FireballTargetPosObservable : public FireballTargetPosObservableBase {
   public:
    enum : size_t { FUNC = 0, DATA };

    void next(WizardId id, SDL_FPoint pos);

    SDL_FPoint getPos(WizardId id) const;

   private:
    void onSubscribe(SubscriptionPtr sub);

    SDL_FPoint mTargets[WizardId::size];
};

class FireballService : public Service<FireballTargetPosObservable> {};

#endif
