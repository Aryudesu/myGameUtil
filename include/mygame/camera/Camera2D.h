#pragma once

#include <mygame/collision/Collision2D.h>

#include <algorithm>

namespace mygame {

class Camera2D {
public:
    void SetPosition(Vec2 position) {
        position_ = position;
        ClampToWorld();
    }
    Vec2 Position() const { return position_; }

    void SetViewport(float width, float height) {
        viewportWidth_ = std::max(0.0f, width);
        viewportHeight_ = std::max(0.0f, height);
        ClampToWorld();
    }

    void SetWorldBounds(const RectF& bounds) {
        worldBounds_ = bounds;
        hasWorldBounds_ = true;
        ClampToWorld();
    }

    void ClearWorldBounds() { hasWorldBounds_ = false; }

    void Follow(Vec2 target, float smoothing = 1.0f) {
        smoothing = std::clamp(smoothing, 0.0f, 1.0f);
        const Vec2 desired{
            target.x - viewportWidth_ * 0.5f,
            target.y - viewportHeight_ * 0.5f
        };
        position_.x += (desired.x - position_.x) * smoothing;
        position_.y += (desired.y - position_.y) * smoothing;
        ClampToWorld();
    }

    Vec2 WorldToScreen(Vec2 world) const {
        return {world.x - position_.x + shakeOffset_.x,
                world.y - position_.y + shakeOffset_.y};
    }

    Vec2 ScreenToWorld(Vec2 screen) const {
        return {screen.x + position_.x - shakeOffset_.x,
                screen.y + position_.y - shakeOffset_.y};
    }

    RectF VisibleWorldRect() const {
        return {position_.x, position_.y, viewportWidth_, viewportHeight_};
    }

    void SetShakeOffset(Vec2 offset) { shakeOffset_ = offset; }
    void ClearShake() { shakeOffset_ = {}; }
    Vec2 ShakeOffset() const { return shakeOffset_; }

private:
    Vec2 position_{};
    Vec2 shakeOffset_{};
    RectF worldBounds_{};
    float viewportWidth_ = 0.0f;
    float viewportHeight_ = 0.0f;
    bool hasWorldBounds_ = false;

    void ClampToWorld() {
        if (!hasWorldBounds_) return;

        const float maxX = std::max(worldBounds_.Left(), worldBounds_.Right() - viewportWidth_);
        const float maxY = std::max(worldBounds_.Top(), worldBounds_.Bottom() - viewportHeight_);
        position_.x = std::clamp(position_.x, worldBounds_.Left(), maxX);
        position_.y = std::clamp(position_.y, worldBounds_.Top(), maxY);
    }
};

} // namespace mygame
