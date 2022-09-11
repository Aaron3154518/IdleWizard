#ifndef POISON_FIREBALL_H
#define POISON_FIREBALL_H

#include <Components/CatalystRing.h>
#include <Components/FireRing.h>
#include <Components/Fireballs/Fireball.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/IconSystem.h>
#include <Systems/TargetSystem.h>
#include <Utils/Number.h>
#include <Wizards/Definitions/PoisonWizardDefs.h>

#include <memory>

class PoisonFireball : public Fireball {
   public:
    struct Data {
        Number power;
        float sizeFactor = .75, speed = .5;
    };

    enum BubbleType : uint8_t { A = 0, B };

    struct Bubble {
        BubbleType type;
        Rect pos;
        SDL_FPoint v;
        int timer;
    };

    typedef TargetSystem::TargetObservable<WizardId, const PoisonFireball&>
        HitObservable;

    class Service : public ::Service<HitObservable> {};

    static std::shared_ptr<HitObservable> GetHitObservable();

   public:
    PoisonFireball(SDL_FPoint c, WizardId target, const Data& data);

    const Number& getPower() const;
    void setPower(const Number& pow);

    void applyTimeEffect(const Number& effect);

   private:
    void init();

    void onUpdate(Time dt);
    void onRender(SDL_Renderer* renderer);

    void onDeath();

    std::mt19937 gen = std::mt19937(rand());
    std::uniform_real_distribution<float> rDist;

    float mSizeSum;
    Number mPower;

    RenderData mBubbleAImg, mBubbleBImg;
    std::vector<Bubble> mBubbles;
};

typedef std::unique_ptr<PoisonFireball> PoisonFireballPtr;

#endif
