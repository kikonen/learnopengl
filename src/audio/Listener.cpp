#include "Listener.h"

#include <fmt/format.h>

#include "util/Log.h"


namespace {
}

namespace audio
{
    Listener::Listener()
    {}

    Listener::Listener(Listener&& b) noexcept
        : m_id{ b.m_id },
        m_gain{ b.m_gain },
        m_pos{ b.m_pos },
        m_vel{ b.m_vel },
        m_front{ b.m_front },
        m_up{ b.m_up }
    {}

    Listener::~Listener()
    {}

    void Listener::prepare()
    {
        // NOTE KI no auto activate
        KI_INFO_OUT(fmt::format("LISTENER: id={}", m_id));
    }

    void Listener::update()
    {
        alListenerf(AL_GAIN, m_gain);

        updatePos();
    }

    void Listener::updatePos()
    {
        alListener3f(AL_POSITION, m_pos.x, m_pos.y, m_pos.z);
        alListener3f(AL_VELOCITY, m_vel.x, m_vel.y, m_vel.z);

        float orientation[6] {
            m_front.x, m_front.y, m_front.z,
            m_up.x, m_up.y, m_up.z
        };
        alListenerfv(AL_ORIENTATION, orientation);
    }
}
