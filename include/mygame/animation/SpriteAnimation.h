#pragma once

#include <mygame/graphics/ImageManager.h>

#include <algorithm>
#include <cstddef>

namespace mygame {

class SpriteAnimation {
public:
    SpriteAnimation() = default;

    SpriteAnimation(ImageManager::ImageId imageId,
                    std::size_t firstFrame,
                    std::size_t frameCount,
                    double secondsPerFrame,
                    bool loop = true) {
        Configure(imageId, firstFrame, frameCount, secondsPerFrame, loop);
    }

    void Configure(ImageManager::ImageId imageId,
                   std::size_t firstFrame,
                   std::size_t frameCount,
                   double secondsPerFrame,
                   bool loop = true) {
        imageId_ = imageId;
        firstFrame_ = firstFrame;
        frameCount_ = frameCount;
        secondsPerFrame_ = std::max(0.0, secondsPerFrame);
        loop_ = loop;
        Reset();
    }

    void Play() {
        if (frameCount_ == 0) return;
        playing_ = true;
        finished_ = false;
    }

    void Pause() { playing_ = false; }

    void Reset() {
        currentFrame_ = 0;
        accumulator_ = 0.0;
        playing_ = false;
        finished_ = false;
    }

    void Restart() {
        Reset();
        Play();
    }

    void Update(double deltaSeconds) {
        if (!playing_ || frameCount_ <= 1 || secondsPerFrame_ <= 0.0) return;

        accumulator_ += std::max(0.0, deltaSeconds);
        while (accumulator_ >= secondsPerFrame_ && playing_) {
            accumulator_ -= secondsPerFrame_;
            AdvanceFrame();
        }
    }

    bool Draw(int x, int y, bool transparent = true) const {
        return ImageManager::GetInstance().Draw(imageId_, x, y, transparent, FrameIndex());
    }

    bool DrawRotated(float x, float y, double scale = 1.0, double angle = 0.0,
                     bool transparent = true) const {
        return ImageManager::GetInstance().DrawRotated(
            imageId_, x, y, scale, angle, transparent, FrameIndex());
    }

    std::size_t FrameIndex() const { return firstFrame_ + currentFrame_; }
    std::size_t LocalFrame() const { return currentFrame_; }
    std::size_t FrameCount() const { return frameCount_; }
    bool Playing() const { return playing_; }
    bool Finished() const { return finished_; }
    bool Loop() const { return loop_; }

private:
    ImageManager::ImageId imageId_ = 0;
    std::size_t firstFrame_ = 0;
    std::size_t frameCount_ = 0;
    std::size_t currentFrame_ = 0;
    double secondsPerFrame_ = 0.1;
    double accumulator_ = 0.0;
    bool loop_ = true;
    bool playing_ = false;
    bool finished_ = false;

    void AdvanceFrame() {
        if (currentFrame_ + 1 < frameCount_) {
            ++currentFrame_;
            return;
        }

        if (loop_) {
            currentFrame_ = 0;
        } else {
            currentFrame_ = frameCount_ - 1;
            playing_ = false;
            finished_ = true;
        }
    }
};

} // namespace mygame
