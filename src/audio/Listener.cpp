#include "Listener.h"

#include <mutex>

#include <fmt/format.h>

#include "util/Log.h"


namespace {
    audio::listener_id idBase{ 0 };

    std::mutex id_lock{};

    audio::listener_id nextID()
    {
        std::lock_guard<std::mutex> lock(id_lock);
        return ++idBase;
    }
}

namespace audio
{
    Listener::Listener()
        : m_id(nextID())
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
