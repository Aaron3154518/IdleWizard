#ifndef FIREBALL_BASE_H
#define FIREBALL_BASE_H

#include <RenderSystem/AssetManager.h>
#include <RenderSystem/RenderTypes.h>
#include <RenderSystem/Shapes.h>
#include <RenderSystem/TextureBuilder.h>
#include <SDL.h>
#include <ServiceSystem/Component.h>
#include <ServiceSystem/CoreServices/RenderService.h>
#include <ServiceSystem/EventServices/ResizeService.h>
#include <ServiceSystem/ServiceSystem.h>
#include <Systems/IconSystem/IconSystem.h>
#include <Systems/TimeSystem.h>
#include <Systems/WizardSystem/WizardObservables.h>
#include <Utils/AnimationData.h>
#include <Utils/Time.h>
#include <Wizards/PoisonWizard/Glob.h>
#include <Wizards/WizardIds.h>

#include <memory>

class FireballBase : public Component {
    friend class FireballListImpl;
    template <class FbT>
    friend class FireballListBase;

   public:
    const static Rect IMG_RECT;
    const static int MAX_SPEED;

    FireballBase(SDL_FPoint c, WizardId target);
    virtual ~FireballBase() = default;

    virtual void draw(TextureBuilder& tex);

    void launch(SDL_FPoint target);

    float getSize() const;
    void setSize(float size);

    float getSpeed() const;
    void setSpeed(float speed);

    Rect getPos() const;
    void setPos(float x, float y);

    void setImg(const RenderTextureCPtr& img);

    WizardId getTargetId() const;

    static bool filter(const FireballBase& fb, WizardId id);

   protected:
    virtual void init();

    virtual void onUpdate(Time dt);

    void move(Time dt);

    WizardId mTargetId;

    float mSize = 1, mSpeed = 1;
    UIComponentPtr mPos;
    SDL_FPoint mV{0, 0}, mA{0, 0};

    RenderData mImg;

    PoisonWizard::Glob::HitObservable::SubscriptionPtr mGlobHitSub;
};

typedef std::unique_ptr<FireballBase> FireballPtr;

typedef std::vector<FireballPtr> FBVector;

class FireballListImpl : public Component {
    friend class FireballObservable;

   public:
    virtual ~FireballListImpl() = default;

    size_t size() const;

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

template <class FbT>
class FireballListBase : public FireballListImpl {
    static_assert(std::is_base_of<FireballBase, FbT>::value,
                  "FireballList template must inherit from FireballBase");

    class iterator {
       public:
        iterator(FBVector::iterator it) : mIt(it) {}

        iterator& operator++() {
            ++mIt;
            return *this;
        }

        bool operator!=(const iterator& rhs) { return mIt != rhs.mIt; }

        FbT& operator*() {
            FbT* ptr = static_cast<FbT*>(mIt->get());
            if (!ptr) {
                throw std::runtime_error(
                    "FireballListImpl could not convert entry to correct "
                    "Fireball type");
            }
            return *ptr;
        }

       private:
        FBVector::iterator mIt;
    };
    class const_iterator {
       public:
        const_iterator(FBVector::const_iterator it) : mIt(it) {}

        const_iterator& operator++() {
            ++mIt;
            return *this;
        }

        bool operator!=(const const_iterator& rhs) { return mIt != rhs.mIt; }

        const FbT& operator*() {
            FbT* const ptr = static_cast<FbT* const>(mIt->get());
            if (!ptr) {
                throw std::runtime_error(
                    "FireballListImpl could not convert entry to correct "
                    "Fireball type");
            }
            return *ptr;
        }

       private:
        FBVector::const_iterator mIt;
    };

   public:
    // Subscribe to collide with fireballs
    typedef Observable<void(const FbT&), bool(const FbT&), UIComponentPtr>
        HitObservableBase;
    class HitObservable : public HitObservableBase {
       public:
        enum { ON_HIT = 0, FILTER, POS };

        using HitObservableBase::subscribe;
        typename HitObservableBase::SubscriptionPtr subscribe(
            std::function<void(const FbT&)> onHit, UIComponentPtr pos) {
            return subscribe(
                onHit, []() { return true; }, pos);
        }

        bool next(const FbT& fb) {
            for (auto sub : *this) {
                if (!sub->template get<FILTER>()(fb)) {
                    continue;
                }

                Rect fbPos = fb.getPos();
                Rect tPos = sub->template get<POS>()->rect;
                float dx = tPos.cX() - fbPos.cX(), dy = tPos.cY() - fbPos.cY();
                if (sqrtf(dx * dx + dy * dy) > 10) {
                    continue;
                }

                sub->template get<ON_HIT>()(fb);
                return true;
            }

            return false;
        }
    };

    static std::shared_ptr<HitObservable> GetHitObservable();

    // Subscribe to get fireball positions
    typedef ForwardObservable<void(const FireballListBase<FbT>&)> PosObservable;

    static std::shared_ptr<PosObservable> GetPosObservable();

    virtual ~FireballListBase() = default;

    iterator begin() { return iterator(mFireballs.begin()); }
    iterator end() { return iterator(mFireballs.end()); }
    const_iterator begin() const { return const_iterator(mFireballs.begin()); }
    const_iterator end() const { return const_iterator(mFireballs.end()); }

    virtual void push_back(std::unique_ptr<FbT> fb) {
        mFireballs.push_back(std::move(fb));
    }

    bool empty() const { return mFireballs.empty(); }

    FbT& back() {
        if (mFireballs.empty()) {
            throw std::runtime_error("FireballListImpl::back(): list is empty");
        }
        return *iterator(mFireballs.end() - 1);
    }
    const FbT& back() const {
        if (mFireballs.empty()) {
            throw std::runtime_error("FireballListImpl::back(): list is empty");
        }
        return *iterator(mFireballs.end() - 1);
    }

   protected:
    virtual void onUpdate(Time dt) {
        auto hitObs = GetHitObservable();
        for (auto it = mFireballs.begin(); it != mFireballs.end();) {
            if (hitObs->next(*iterator(it))) {
                it = mFireballs.erase(it);
            } else {
                (*it)->onUpdate(dt);
                ++it;
            }
        }
        GetPosObservable()->next(*this);
    }
};

template <class FbT>
class FireballService
    : public Service<typename FireballListBase<FbT>::HitObservable,
                     typename FireballListBase<FbT>::PosObservable> {};

template <class FbT>
inline std::shared_ptr<typename FireballListBase<FbT>::HitObservable>
FireballListBase<FbT>::GetHitObservable() {
    return ServiceSystem::Get<FireballService<FbT>,
                              FireballListBase<FbT>::HitObservable>();
}

template <class FbT>
inline std::shared_ptr<typename FireballListBase<FbT>::PosObservable>
FireballListBase<FbT>::GetPosObservable() {
    return ServiceSystem::Get<FireballService<FbT>,
                              FireballListBase<FbT>::PosObservable>();
}

#endif