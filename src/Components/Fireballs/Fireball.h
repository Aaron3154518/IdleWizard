#ifndef FIREBALL_H
#define FIREBALL_H

#include <Components/Fireballs/Glob.h>
#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <SDL.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/ResizeService.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/IconSystem.h>
#include <Systems/TimeSystem.h>
#include <Systems/WizardSystem.h>
#include <Utils/AnimationData.h>
#include <Utils/Time.h>
#include <Wizards/WizardIds.h>

#include <memory>

class Fireball : public Component {
    friend class FireballList;

   public:
    const static Rect IMG_RECT;
    const static int COLLIDE_ERR, MAX_SPEED;

    Fireball(SDL_FPoint c, WizardId target, float maxSpeedMult,
             const std::string& img);
    Fireball(SDL_FPoint c, WizardId target, float maxSpeedMult,
             const AnimationData& img);
    virtual ~Fireball() = default;

    void launch(SDL_FPoint target);

    float getSize() const;
    void setSize(float size);

    float getSpeed() const;
    void setSpeed(float speed);

    void setPos(float x, float y);

    void setImg(const RenderTextureCPtr& img);

    WizardId getTargetId() const;

   protected:
    virtual void init();

    virtual bool onUpdate(Time dt);
    virtual void onRender(TextureBuilder& tex);
    virtual void onDeath();

    WizardId mTargetId;

    float mSize = 1, mMaxSpeed;
    UIComponentPtr mPos;
    SDL_FPoint mV{0, 0}, mA{0, 0};

    RenderData mImg;

    Glob::HitObservable::SubscriptionPtr mGlobHitSub;
};

typedef std::unique_ptr<Fireball> FireballPtr;

typedef std::vector<FireballPtr> FBVector;

class FireballList : public Component {
    friend class FireballObservable;

   public:
    virtual ~FireballList() = default;

    FBVector::iterator begin();
    FBVector::iterator end();

    size_t size() const;

    FireballPtr& back();

    void add(FireballPtr fb);

    void remove(WizardId target);

    void clear();

   protected:
    virtual void init();

    virtual void onUpdate(Time dt);
    virtual void onResize(ResizeData data);
    virtual void onRender(SDL_Renderer* renderer);

    ResizeObservable::SubscriptionPtr mResizeSub;
    TimeSystem::UpdateObservable::SubscriptionPtr mUpdateSub;
    RenderObservable::SubscriptionPtr mRenderSub;

    FBVector mFireballs;
};

template <class T>
class FireballListReal : public FireballList {
    static_assert(std::is_base_of<Fireball, T>::value,
                  "FireballList template must inherit from Fireball");

    class Iterator {
       public:
        Iterator(FBVector::iterator it) : mIt(it) {}

        Iterator& operator++() {
            ++mIt;
            return *this;
        }

        bool operator!=(const Iterator& rhs) { return mIt != rhs.mIt; }

        T& operator*() {
            T* ptr = std::static_cast < T * (*mIt);
            if (!ptr) {
                throw std::runtime_error(
                    "FireballList could not convert entry to correct Fireball "
                    "type");
            }
            return *ptr;
        }

       private:
        FBVector::iterator mIt;
    };
    typedef Iterator iterator;

   public:
    iterator begin() { return iterator(mFireballs.begin()); }
    iterator end() { return iterator(mFireballs.end()); }

    void add(std::unique_ptr<T> fb) { mFireballs.push_back(std::move(fb)); }

   private:
};

template <class T>
using FireballListPtr = std::unique_ptr<FireballList<T>>;

#endif