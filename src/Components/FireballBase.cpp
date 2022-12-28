#include "FireballBase.h"

// FireballBase
const int FireballBase::MAX_SPEED = 150;

const Rect FireballBase::IMG_RECT(0, 0, 40, 40);

constexpr int FIREBALL_BASE_ROT_DEG = -45;

FireballBase::FireballBase(SDL_FPoint c, WizardId target)
    : mPos(std::make_shared<UIComponent>(Rect(), Elevation::PROJECTILES)),
      mTargetId(target) {
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
    float maxSpeed = mSpeed * MAX_SPEED;

    SDL_FPoint target =
        WizardSystem::GetWizardPos(mTargetId).getPos(Rect::Align::CENTER);
    float dx = target.x - mPos->rect.cX(), dy = target.y - mPos->rect.cY();
    float d = fminf(sqrtf(dx * dx + dy * dy) / 2, maxSpeed);
    float v_squared = fmaxf(mV.x * mV.x + mV.y * mV.y, maxSpeed * maxSpeed / 4);
    float frac = v_squared / (d * d);
    mA.x = dx * frac;
    mA.y = dy * frac;

    float moveX = mV.x * sec + mA.x * aCoeff,
          moveY = mV.y * sec + mA.y * aCoeff;
    mV.x += mA.x * sec;
    mV.y += mA.y * sec;

    // Cap speed
    float mag = sqrtf(mV.x * mV.x + mV.y * mV.y);
    if (mag > maxSpeed) {
        frac = maxSpeed / mag;
        mV.x *= frac;
        mV.y *= frac;
    }

    float theta = 0;
    if (moveX != 0) {
        theta = atanf(moveY / moveX) / DEG_TO_RAD;
        if (moveX < 0) {
            theta += 180;
        }
    }
    theta += FIREBALL_BASE_ROT_DEG;

    mImg.setRotationDeg(theta);
    Rect imgR = mImg.getRect();
    imgR.move(moveX, moveY);
    mImg.setDest(imgR);
    mPos->rect = mImg.getDest();
}

void FireballBase::draw(TextureBuilder& tex) { tex.draw(mImg); }

void FireballBase::launch(SDL_FPoint target) {
    // Start at half max speed
    float dx = target.x - mPos->rect.cX(), dy = target.y - mPos->rect.cY();
    float frac = mSpeed * MAX_SPEED / 3 / sqrtf(dx * dx + dy * dy);
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

float FireballBase::getSpeed() const { return mSpeed; }
void FireballBase::setSpeed(float speed) { mSpeed = speed; }

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
    mResizeSub =
        ServiceSystem::Get<ResizeService, ResizeObservable>()->subscribe(
            [this](ResizeData d) { onResize(d); });
    mUpdateSub = TimeSystem::GetUpdateObservable()->subscribe(
        [this](Time dt) { onUpdate(dt); });
    mRenderSub =
        ServiceSystem::Get<RenderService, RenderObservable>()->subscribe(
            [this](SDL_Renderer* r) { onRender(r); },
            std::make_shared<UIComponent>(Rect(), Elevation::PROJECTILES));
}

void FireballListImpl::onResize(ResizeData data) {
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
