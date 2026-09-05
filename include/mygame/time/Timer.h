#pragma once

#include <algorithm>
#include <chrono>

namespace mygame {

class Timer {
public:
    using Clock = std::chrono::steady_clock;

    explicit Timer(double durationSeconds = 0.0)
        : duration_(std::max(0.0, durationSeconds)) {}

    void Start(double durationSeconds) {
        duration_ = std::max(0.0, durationSeconds);
        elapsed_ = 0.0;
        running_ = true;
        finishedThisUpdate_ = false;
    }

    void Start() {
        elapsed_ = 0.0;
        running_ = true;
        finishedThisUpdate_ = false;
    }

    void Stop() {
        running_ = false;
        finishedThisUpdate_ = false;
    }

    void Reset() {
        elapsed_ = 0.0;
        running_ = false;
        finishedThisUpdate_ = false;
    }

    void Update(double deltaSeconds) {
        finishedThisUpdate_ = false;
        if (!running_) return;

        elapsed_ += std::max(0.0, deltaSeconds);
        if (elapsed_ >= duration_) {
            elapsed_ = duration_;
            running_ = false;
            finishedThisUpdate_ = true;
        }
    }

    bool Running() const { return running_; }
    bool Finished() const { return !running_ && elapsed_ >= duration_; }
    bool FinishedThisUpdate() const { return finishedThisUpdate_; }

    double Duration() const { return duration_; }
    double Elapsed() const { return elapsed_; }
    double Remaining() const { return std::max(0.0, duration_ - elapsed_); }
    double Progress() const {
        if (duration_ <= 0.0) return Finished() ? 1.0 : 0.0;
        return std::clamp(elapsed_ / duration_, 0.0, 1.0);
    }

private:
    double duration_ = 0.0;
    double elapsed_ = 0.0;
    bool running_ = false;
    bool finishedThisUpdate_ = false;
};

class Cooldown {
public:
    explicit Cooldown(double seconds = 0.0) : timer_(seconds) {}

    bool Ready() const { return !timer_.Running(); }

    bool TryUse() {
        if (!Ready()) return false;
        timer_.Start();
        return true;
    }

    bool TryUse(double seconds) {
        if (!Ready()) return false;
        timer_.Start(seconds);
        return true;
    }

    void Update(double deltaSeconds) { timer_.Update(deltaSeconds); }
    void Reset() { timer_.Reset(); }
    double Remaining() const { return timer_.Remaining(); }
    double Progress() const { return timer_.Progress(); }

private:
    Timer timer_;
};

class DeltaClock {
public:
    DeltaClock() : previous_(Clock::now()) {}

    double Tick() {
        const auto now = Clock::now();
        const std::chrono::duration<double> delta = now - previous_;
        previous_ = now;
        return delta.count();
    }

    void Reset() { previous_ = Clock::now(); }

private:
    using Clock = std::chrono::steady_clock;
    Clock::time_point previous_;
};

} // namespace mygame
