#pragma once

#include <random>

namespace util
{
    class Random {
    public:
        Random(int seed);
        ~Random();

        // @return float in range [0, 1]
        float rnd() const;
        float rnd(float max) const;

    private:
        mutable std::mt19937 m_rng;
    };
}
