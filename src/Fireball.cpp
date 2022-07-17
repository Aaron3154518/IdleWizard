#include "Fireball.h"

// FireballObservable
FireballObservable::SubscriptionPtr FireballObservable::subscribe(
    Subscription::Function func, std::shared_ptr<WizardId> id) {
    func(mTargets[*id]);
    return FireballObservableBase::subscribe(func, id);
}

void FireballObservable::updateSubscriptionData(SubscriptionPtr sub,
                                                std::shared_ptr<WizardId> id) {
    if (*id == WizardId::size) {
        throw std::runtime_error("Cannot use WizardId::size as a target");
    }
    FireballObservableBase::updateSubscriptionData(sub, id);
    (*sub)(mTargets[*id]);
}

void FireballObservable::next(WizardId id, SDL_FPoint pos) {
    removeUnsubscribed();

    if (id == WizardId::size) {
        throw std::runtime_error("Cannot use WizardId::size as a target");
    }
    mTargets[id] = pos;
    for (auto sub : mSubscriptions) {
        if (*sub->getData() == id) {
            (*sub)(pos);
        }
    }
}

// TargetObservable
void TargetObservable::next(WizardId target, Number val, WizardId src) {
    removeUnsubscribed();

    for (auto sub : mSubscriptions) {
        if (*sub->getData() == target) {
            (*sub)(src, val);
        }
    }
}

// Fireball
const int Fireball::MAX_SPEED = 150;
const int Fireball::COLLIDE_ERR = 10;
const std::string Fireball::IMG = "res/projectiles/fireball.png";

Fireball::Fireball(float cX, float cY, WizardId src, WizardId target,
                   Number val)
    : mComp(std::make_shared<UIComponent>(Rect(), 0)),
      mTargetId(std::make_shared<WizardId>(target)),
      mSrcId(src),
      mVal(val) {
    mImg.texture = AssetManager::getTexture(IMG);
    mImg.dest = Rect(0, 0, 50, 50);
    mImg.fitToTexture();
    mImg.dest.setPos(cX, cY, Rect::Align::CENTER);
    mComp->rect = mImg.dest;
}

void Fireball::init() {
    mUpdateSub =
        ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
            std::bind(&Fireball::onUpdate, this, std::placeholders::_1));
    mUpdateSub->setUnsubscriber(unsub);
    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            std::bind(&Fireball::onRender, this, std::placeholders::_1), mComp);
    mRenderSub->setUnsubscriber(unsub);
    mFireballSub =
        ServiceSystem::Get<FireballService, FireballObservable>()->subscribe(
            [this](SDL_FPoint p) { mTargetPos = p; }, mTargetId);
    mFireballSub->setUnsubscriber(unsub);
}

bool Fireball::dead() const { return unsub; }

void Fireball::onUpdate(Time dt) {
    float dx = mTargetPos.x - mComp->rect.cX(),
          dy = mTargetPos.y - mComp->rect.cY();
    float mag = std::sqrt(dx * dx + dy * dy);
    // Check in case we were moved onto the target
    if (mag < COLLIDE_ERR) {
        unsub.unsubscribe();
        ServiceSystem::Get<FireballService, TargetObservable>()->next(
            *mTargetId, mVal, mSrcId);
    } else {
        mComp->rect.move(MAX_SPEED * dt.s(), dx, dy);
        mImg.dest = mComp->rect;
    }
}

void Fireball::onRender(SDL_Renderer* renderer) { TextureBuilder().draw(mImg); }
