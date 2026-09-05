#pragma once

#include <algorithm>
#include <cmath>

namespace mygame {

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2 operator+(const Vec2& other) const { return {x + other.x, y + other.y}; }
    Vec2 operator-(const Vec2& other) const { return {x - other.x, y - other.y}; }
    Vec2 operator*(float scale) const { return {x * scale, y * scale}; }
};

struct RectF {
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;

    float Left() const { return x; }
    float Right() const { return x + width; }
    float Top() const { return y; }
    float Bottom() const { return y + height; }
    Vec2 Center() const { return {x + width * 0.5f, y + height * 0.5f}; }
};

struct CircleF {
    Vec2 center{};
    float radius = 0.0f;
};

namespace collision {

inline bool Contains(const RectF& rect, const Vec2& point) {
    return point.x >= rect.Left() && point.x <= rect.Right() &&
           point.y >= rect.Top() && point.y <= rect.Bottom();
}

inline bool Contains(const CircleF& circle, const Vec2& point) {
    const float dx = point.x - circle.center.x;
    const float dy = point.y - circle.center.y;
    return dx * dx + dy * dy <= circle.radius * circle.radius;
}

inline bool Intersects(const RectF& a, const RectF& b) {
    return a.Left() <= b.Right() && a.Right() >= b.Left() &&
           a.Top() <= b.Bottom() && a.Bottom() >= b.Top();
}

inline bool Intersects(const CircleF& a, const CircleF& b) {
    const float dx = a.center.x - b.center.x;
    const float dy = a.center.y - b.center.y;
    const float radius = a.radius + b.radius;
    return dx * dx + dy * dy <= radius * radius;
}

inline bool Intersects(const RectF& rect, const CircleF& circle) {
    const float nearestX = std::clamp(circle.center.x, rect.Left(), rect.Right());
    const float nearestY = std::clamp(circle.center.y, rect.Top(), rect.Bottom());
    const float dx = circle.center.x - nearestX;
    const float dy = circle.center.y - nearestY;
    return dx * dx + dy * dy <= circle.radius * circle.radius;
}

inline bool Intersects(const CircleF& circle, const RectF& rect) {
    return Intersects(rect, circle);
}

inline float DistanceSquared(const Vec2& a, const Vec2& b) {
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    return dx * dx + dy * dy;
}

inline float Distance(const Vec2& a, const Vec2& b) {
    return std::sqrt(DistanceSquared(a, b));
}

} // namespace collision
} // namespace mygame
