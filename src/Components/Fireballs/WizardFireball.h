#ifndef WIZARD_FIREBALL_H
#define WIZARD_FIREBALL_H

#include <Components/CatalystRing.h>
#include <Components/FireRing.h>
#include <Components/Fireballs/Fireball.h>
#include <Wizards/Definitions/WizardDefs.h>

#include <random>

class WizardFireball : public Fireball {
    friend class WizardFireballList;

   public:
    struct Data {
        Number power;
        float sizeFactor = 1, speed = 1;
        bool boosted = false, poisoned = false;
    };

    WizardFireball(SDL_FPoint c, WizardId target, const Data& data);

    void draw(TextureBuilder& tex);

    Data getData() const;

    bool isBoosted() const;
    bool isPoisoned() const;

    const Number& getPower() const;
    void setPower(const Number& pow);

    void addFireball(const Data& data);

    void applyTimeEffect(const Number& effect);

    void updateImage();

   private:
    void init();

    void onFireRingHit(const Number& effect);
    void onCatalystHit(bool poisoned);

    std::unique_ptr<WizardFireball> split();

    void setOnPoisoned(
        std::function<void(std::unique_ptr<WizardFireball>)> push_back);

    Number mPower;

    bool mHitFireRing = false, mBoosted = false, mPoisoned = false;

    float mSizeSum;

    RenderData mOuterImg;

    std::mt19937 gen = std::mt19937(rand());
    std::uniform_real_distribution<float> rDist;

    FireRing::HitObservable::SubscriptionPtr mFireRingSub;
    CatalystRing::HitObservable::SubscriptionPtr mCatalystHitSub;
    Glob::HitObservable::SubscriptionPtr mGlobHitSub;

    std::function<void()> mOnPoisoned;
};

typedef std::unique_ptr<WizardFireball> WizardFireballPtr;

class WizardFireballList : public FireballList<WizardFireball> {
   public:
    void push_back(WizardFireballPtr fb);

   private:
};

typedef std::unique_ptr<WizardFireballList> WizardFireballListPtr;

#endif
