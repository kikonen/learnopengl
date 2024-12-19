#include "Random.h"

namespace {
    constexpr int RANGE = INT_MAX;

    std::random_device dev;
    std::uniform_int_distribution<std::mt19937::result_type> uniform_dist{ 0, RANGE };
}

namespace util {
    Random::Random(int seed)
        : m_rng{ dev() }
    {
        m_rng.seed(seed);
    }

    Random::~Random() = default;

    float Random::rnd() const
    {
        return static_cast<float>(uniform_dist(m_rng)) / (float)RANGE;
    }

    float Random::rnd(float max) const
    {
        return max* rnd();
    }
}
