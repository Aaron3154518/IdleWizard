#include "FireballBase.h"

// FireballBase
const int FireballBase::BASE_SPEED = 150;

const Rect FireballBase::IMG_RECT(0, 0, 40, 40);

constexpr int FIREBALL_BASE_ROT_DEG = -45;

FireballBase::FireballBase(SDL_FPoint c, WizardId target)
    : mTargetId(target),
      mPos(std::make_shared<UIComponent>(Rect(), Elevation::PROJECTILES)) {
    mImg.setDest(Rect(c.x, c.y, 0, 0));
    mPos->rect = mImg.getDest();
}

void FireballBase::init() {
    launch(WizardSystem::GetWizardPos(mTargetId).getPos(Rect::Align::CENTER));
}

void FireballBase::onUpdate(Time dt) { move(dt); }

void FireballBase::move(Time dt) {
    float sec = dt.s();
    float aCoeff = sec * sec / 2;
    float maxSpeed = getSpeed();

    SDL_FPoint target =
        WizardSystem::GetWizardPos(mTargetId).getPos(Rect::Align::CENTER);
    float dx = target.x - mPos->rect.cX(), dy = target.y - mPos->rect.cY();
    float d = fminf(sqrtf(dx * dx + dy * dy) / 2, maxSpeed);

    if (fabsf(d) < 1e-5 || fabsf(maxSpeed) < 1e-5) {
        mImg.setRotationDeg(FIREBALL_BASE_ROT_DEG);
        return;
    }

    float v_squared = fmaxf(mV.x * mV.x + mV.y * mV.y, maxSpeed * maxSpeed / 4);
    float frac = v_squared / (d * d);
    mA.x = dx * frac;
    mA.y = dy * frac;

    float moveX = Math::min_mag(dx, mV.x * sec + mA.x * aCoeff),
          moveY = Math::min_mag(dy, mV.y * sec + mA.y * aCoeff);
    mV.x += mA.x * sec;
    mV.y += mA.y * sec;

    // Cap speed
    float mag = sqrtf(mV.x * mV.x + mV.y * mV.y);
    if (mag > maxSpeed) {
        frac = maxSpeed / mag;
        mV.x *= frac;
        mV.y *= frac;
    }

    float theta = atan2f(moveY, moveX) / DEG_TO_RAD;
    theta += FIREBALL_BASE_ROT_DEG;

    mImg.setRotationDeg(theta);
    Rect imgR = mImg.getRect();
    imgR.move(moveX, moveY);
    mImg.setDest(imgR);
    mPos->rect = mImg.getDest();
}

void FireballBase::draw(TextureBuilder& tex) { tex.draw(mImg); }

void FireballBase::launch(SDL_FPoint target) {
    // Start at third max speed
    float dx = target.x - mPos->rect.cX(), dy = target.y - mPos->rect.cY();
    float frac = getSpeed() / 3 / sqrtf(dx * dx + dy * dy);
    mV.x = dx * frac;
    mV.y = dy * frac;
}

float FireballBase::getSize() const { return mSize; }
void FireballBase::setSize(float size) {
    mSize = fminf(fmaxf(size, 0), 50);
    Rect imgR = mImg.getRect();
    imgR.setDim(IMG_RECT.w() * mSize, IMG_RECT.h() * mSize,
                Rect::Align::CENTER);
    mImg.setDest(imgR);
    mPos->rect = mImg.getDest();
}

float FireballBase::getSpeed() const { return mSpeed * BASE_SPEED; }
float FireballBase::getSpeedFactor() const { return mSpeed; }
void FireballBase::setSpeedFactor(float speed) { mSpeed = speed; }

Rect FireballBase::getPos() const { return mPos->rect; }
void FireballBase::setPos(float x, float y) {
    Rect imgR = mImg.getRect();
    imgR.setPos(x, y, Rect::Align::CENTER);
    mImg.setDest(imgR);
    mPos->rect = mImg.getDest();
}

WizardId FireballBase::getTargetId() const { return mTargetId; }

void FireballBase::setImg(const RenderTextureCPtr& img) {
    mImg.set(img);
    mPos->rect = mImg.getDest();
}

bool FireballBase::filter(const FireballBase& fb, WizardId id) {
    return fb.getTargetId() == id;
}

// FireballListImpl
size_t FireballListImpl::size() const { return mFireballs.size(); }

void FireballListImpl::remove(WizardId target) {
    std::remove_if(mFireballs.begin(), mFireballs.end(),
                   [target](const FireballPtr& fb) {
                       return fb->getTargetId() == target;
                   });
}

void FireballListImpl::clear() { mFireballs.clear(); }

void FireballListImpl::init() {
    mResizeSub = EventServices::GetResizeObservable()->subscribe(
        [this](EventServices::ResizeData d) { onResize(d); });
    mUpdateSub = TimeSystem::GetUpdateObservable()->subscribe(
        [this](Time dt) { onUpdate(dt); });
    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [this](SDL_Renderer* r) { onRender(r); },
            std::make_shared<UIComponent>(Rect(), Elevation::PROJECTILES));
}

void FireballListImpl::onResize(EventServices::ResizeData data) {
    for (auto& fb : mFireballs) {
        fb->mPos->rect.moveFactor((float)data.newW / data.oldW,
                                  (float)data.newH / data.oldH);
    }
}

void FireballListImpl::onUpdate(Time dt) {}

void FireballListImpl::onRender(SDL_Renderer* renderer) {
    TextureBuilder tex;
    for (auto& fb : mFireballs) {
        fb->draw(tex);
    }
}
