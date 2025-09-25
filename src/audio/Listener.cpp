#include "Listener.h"

#include <fmt/format.h>

#include "util/Log.h"

#include "model/NodeState.h"

namespace {
}

namespace audio
{
    Listener::Listener(Listener&& o) noexcept
        : m_gain{ o.m_gain }
    {}

    Listener::~Listener()
    {}

    Listener& Listener::operator=(Listener&& o) noexcept
    {
        if (&o == this) return *this;

        m_gain = o.m_gain;

        return *this;
    }

    void Listener::updateActive(const model::NodeState& state) const
    {
        const auto& pos = state.getWorldPosition();
        const auto& front = glm::normalize(state.getViewFront());
        const auto& up = glm::normalize(state.getViewUp());
        const auto& vel = glm::vec3{ 0.f };

        alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
        alListener3f(AL_VELOCITY, vel.x, vel.y, vel.z);

        float orientation[6]{
            front.x, front.y, front.z,
            up.x, up.y, up.z
        };
        alListenerfv(AL_ORIENTATION, orientation);
        alListenerf(AL_GAIN, m_gain);
    }
}
