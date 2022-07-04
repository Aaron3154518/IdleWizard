#include "Fireball.h"

// FireballObservable
FireballObservable::SubscriptionPtr FireballObservable::subscribe(
    Subscription::Function func, std::shared_ptr<WizardId> id) {
    func(mTargets[*id]);
    return FireballObservableBase::subscribe(func, id);
}

void FireballObservable::updateSubscriptionData(SubscriptionPtr sub, std::shared_ptr<WizardId> id) {
    if (*id == WizardId::size) {
        throw std::runtime_error("Cannot use WizardId::size as a target");
    }
    FireballObservableBase::updateSubscriptionData(sub, id);
    (*sub)(mTargets[*id]);
}

void FireballObservable::next(WizardId id, SDL_Point pos) {
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

// Fireball
const int Fireball::MAX_SPEED = 150;
const int Fireball::COLLIDE_ERR = 10;
const std::string Fireball::IMG = "res/projectiles/fireball.png";

Fireball::Fireball(SDL_Point c, WizardId target) : mComp(std::make_shared<UIComponent>(Rect(), 0)),
                                                   mTargetId(std::make_shared<WizardId>(target)) {
    mImg.texture = AssetManager::getTexture(IMG);
    mImg.dest = Rect(0, 0, 50, 50);
    mImg.fitToTexture();
    setPos((float)c.x, (float)c.y);
}

void Fireball::init() {
    mUpdateSub = ServiceSystem::Get<UpdateService, UpdateObservable>()->subscribe(
        std::bind(&Fireball::onUpdate, this, std::placeholders::_1));
    mUpdateSub->setUnsubscriber(unsub);
    mRenderSub = ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
        std::bind(&Fireball::onRender, this, std::placeholders::_1), mComp);
    mRenderSub->setUnsubscriber(unsub);
    mFireballSub = ServiceSystem::Get<FireballService, FireballObservable>()->subscribe(
        [this](SDL_Point p) { mTargetPos = p; }, mTargetId);
    mFireballSub->setUnsubscriber(unsub);
}

bool Fireball::dead() const {
    return unsub;
}

void Fireball::setPos(float x, float y) {
    mPos.x = x;
    mPos.y = y;
    mImg.dest.setCenter(mPos.x, mPos.y);
    mComp->rect = mImg.dest;
}

void Fireball::onUpdate(Time dt) {
    double dx = mTargetPos.x - mComp->rect.cX(),
           dy = mTargetPos.y - mComp->rect.cY();
    double mag = std::sqrt(dx * dx + dy * dy);
    // Check in case we were moved onto the target
    if (std::abs(mag) < COLLIDE_ERR) {
        unsub.unsubscribe();
        return;
    }

    double vx = MAX_SPEED * dx / mag,
           vy = MAX_SPEED * dy / mag;
    setPos(mPos.x + vx * dt.s(), mPos.y + vy * dt.s());

    dx = mTargetPos.x - mPos.x;
    dy = mTargetPos.y - mPos.y;
    mag = std::sqrt(dx * dx + dy * dy);
    if (std::abs(mag) < COLLIDE_ERR) {
        unsub.unsubscribe();
    }
}

void Fireball::onRender(SDL_Renderer* renderer) {
    TextureBuilder().draw(mImg);
}
