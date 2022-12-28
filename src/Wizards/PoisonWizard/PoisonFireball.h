#ifndef POISON_FIREBALL_H
#define POISON_FIREBALL_H

#include <Wizards/Catalyst/CatalystRing.h>
#include <Wizards/Crystal/FireRing.h>
#include <Components/FireballBase.h>
#include <ServiceSystem/Observable.h>
#include <ServiceSystem/Service.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/IconSystem/IconSystem.h>
#include <Systems/TargetSystem.h>
#include <Utils/Number.h>
#include <Wizards/PoisonWizard/PoisonWizardConstants.h>

#include <memory>

namespace PoisonWizard {
class Fireball : public FireballBase {
   public:
    struct Data {
        Number power, duration;
        float sizeFactor = .75, speed = .5;
    };

    enum BubbleType : uint8_t { A = 0, B };

    struct Bubble {
        BubbleType type;
        Rect pos;
        SDL_FPoint v;
        int timer;
    };

   public:
    Fireball(SDL_FPoint c, WizardId target, const Data& data);

    void draw(TextureBuilder& tex);

    const Number& getPower() const;
    void setPower(const Number& pow);

    const Number& getDuration() const;
    void setDuration(const Number& duration);

    void applyTimeEffect(const Number& effect);

   private:
    void init();

    void onUpdate(Time dt);

    std::mt19937 gen = std::mt19937(rand());
    std::uniform_real_distribution<float> rDist;

    float mSizeSum;
    Number mPower, mDuration;

    RenderData mBubbleAImg, mBubbleBImg;
    std::vector<Bubble> mBubbles;
};

typedef std::unique_ptr<Fireball> FireballPtr;

typedef FireballListBase<Fireball> FireballList;
typedef std::unique_ptr<FireballList> FireballListPtr;
}  // namespace PoisonWizard

#endif
