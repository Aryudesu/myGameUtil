#pragma once

#include <algorithm>
#include <cmath>

namespace mygame::easing {

inline double Clamp01(double t) {
    return std::clamp(t, 0.0, 1.0);
}

inline double Linear(double t) {
    return Clamp01(t);
}

inline double EaseInQuad(double t) {
    t = Clamp01(t);
    return t * t;
}

inline double EaseOutQuad(double t) {
    t = Clamp01(t);
    return 1.0 - (1.0 - t) * (1.0 - t);
}

inline double EaseInOutQuad(double t) {
    t = Clamp01(t);
    return t < 0.5 ? 2.0 * t * t : 1.0 - std::pow(-2.0 * t + 2.0, 2.0) / 2.0;
}

inline double EaseInCubic(double t) {
    t = Clamp01(t);
    return t * t * t;
}

inline double EaseOutCubic(double t) {
    t = Clamp01(t);
    return 1.0 - std::pow(1.0 - t, 3.0);
}

inline double EaseInOutCubic(double t) {
    t = Clamp01(t);
    return t < 0.5 ? 4.0 * t * t * t
                   : 1.0 - std::pow(-2.0 * t + 2.0, 3.0) / 2.0;
}

inline double EaseOutBack(double t, double overshoot = 1.70158) {
    t = Clamp01(t);
    const double c3 = overshoot + 1.0;
    return 1.0 + c3 * std::pow(t - 1.0, 3.0) + overshoot * std::pow(t - 1.0, 2.0);
}

template <class T>
inline T Lerp(const T& from, const T& to, double t) {
    return static_cast<T>(from + (to - from) * Clamp01(t));
}

} // namespace mygame::easing
