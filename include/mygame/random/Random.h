#pragma once

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <random>

namespace mygame {

class Random {
public:
    explicit Random(std::uint32_t seed = std::random_device{}()) : engine_(seed) {}

    void Seed(std::uint32_t seed) { engine_.seed(seed); }

    int Int(int minValue, int maxValue) {
        std::uniform_int_distribution<int> distribution(minValue, maxValue);
        return distribution(engine_);
    }

    std::uint32_t UInt(std::uint32_t minValue, std::uint32_t maxValue) {
        std::uniform_int_distribution<std::uint32_t> distribution(minValue, maxValue);
        return distribution(engine_);
    }

    double Real(double minValue = 0.0, double maxValue = 1.0) {
        std::uniform_real_distribution<double> distribution(minValue, maxValue);
        return distribution(engine_);
    }

    bool Chance(double probability) {
        probability = std::clamp(probability, 0.0, 1.0);
        std::bernoulli_distribution distribution(probability);
        return distribution(engine_);
    }

    template <class RandomIt>
    void Shuffle(RandomIt first, RandomIt last) {
        std::shuffle(first, last, engine_);
    }

    template <class Container>
    auto& Choice(Container& container) {
        auto first = std::begin(container);
        const auto size = std::distance(first, std::end(container));
        std::advance(first, Int(0, static_cast<int>(size - 1)));
        return *first;
    }

    template <class Container>
    const auto& Choice(const Container& container) {
        auto first = std::begin(container);
        const auto size = std::distance(first, std::end(container));
        std::advance(first, Int(0, static_cast<int>(size - 1)));
        return *first;
    }

private:
    std::mt19937 engine_;
};

} // namespace mygame
