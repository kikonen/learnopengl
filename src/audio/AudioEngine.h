#pragma once

#include <string>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "AL/alc.h"

#include "size.h"

struct UpdateContext;

namespace audio
{
    class SoundRegistry;

    struct Listener;
    struct Source;

    //
    // https://indiegamedev.net/2020/02/15/the-complete-guide-to-openal-with-c-part-1-playing-a-sound/
    //
    class AudioEngine {
    public:
        static AudioEngine& get() noexcept;

        AudioEngine();
        AudioEngine& operator=(const AudioEngine&) = delete;

        ~AudioEngine();

        void prepare();

        inline bool isEnabled(bool enabled) const {
            return m_enabled;
        }

        inline void setEnabled(bool enabled) {
            m_enabled = enabled;
        }

        void setActiveListenerId(ki::node_id nodeId);

        ki::node_id getActiveListenerId() const noexcept
        {
            return m_activeListenerId;
        }

        bool isActiveListener(ki::node_id nodeId) const noexcept
        {
            return m_activeListenerId == nodeId;
        }

        void prepareSource(audio::Source& source);

        audio::sound_id registerSound(std::string_view fullPath);

    private:
        bool m_prepared{ false };
        bool m_enabled{ false };

        ALCdevice* m_device{ nullptr };
        ALCcontext* m_context{ nullptr };

        ki::node_id m_activeListenerId{ 0 };

        std::unique_ptr<SoundRegistry> m_soundRegistry;
    };
}
