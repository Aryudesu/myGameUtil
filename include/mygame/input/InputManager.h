#pragma once

#include <DxLib.h>

#include <array>
#include <cstdint>
#include <limits>

namespace mygame {

class InputManager {
public:
    static constexpr int KeyCount = 256;

    static InputManager& GetInstance() {
        static InputManager instance;
        return instance;
    }

    bool Update() {
        std::array<char, KeyCount> raw{};
        if (GetHitKeyStateAll(raw.data()) != 0) {
            return false;
        }

        previous_ = current_;
        for (int key = 0; key < KeyCount; ++key) {
            current_[key] = raw[key] != 0;
            if (current_[key]) {
                if (holdFrames_[key] < std::numeric_limits<std::uint32_t>::max()) {
                    ++holdFrames_[key];
                }
            } else {
                holdFrames_[key] = 0;
            }
        }
        return true;
    }

    bool IsDown(int key) const {
        return IsValidKey(key) && current_[key];
    }

    bool IsPressed(int key) const {
        return IsValidKey(key) && current_[key] && !previous_[key];
    }

    bool IsReleased(int key) const {
        return IsValidKey(key) && !current_[key] && previous_[key];
    }

    std::uint32_t HoldFrames(int key) const {
        return IsValidKey(key) ? holdFrames_[key] : 0;
    }

    void Reset() {
        current_.fill(false);
        previous_.fill(false);
        holdFrames_.fill(0);
    }

private:
    static constexpr bool IsValidKey(int key) {
        return 0 <= key && key < KeyCount;
    }

    std::array<bool, KeyCount> current_{};
    std::array<bool, KeyCount> previous_{};
    std::array<std::uint32_t, KeyCount> holdFrames_{};
};

} // namespace mygame
