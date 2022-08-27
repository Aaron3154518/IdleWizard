#ifndef FIREBALL_TARGET_OBSERVABLE_H
#define FIREBALL_TARGET_OBSERVABLE_H

#include <SDL.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <Systems/TargetSystem.h>
#include <Wizards/WizardIds.h>

typedef TargetSystem::TargetDataObservable<SDL_FPoint>
    FireballTargetPosObservable;

class FireballService : public Service<FireballTargetPosObservable> {};

#endif
