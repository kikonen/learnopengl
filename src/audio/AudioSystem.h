#pragma once

#include <string>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "al_call.h"

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
    class AudioSystem {
    public:
        static void init() noexcept;
        static void release() noexcept;
        static AudioSystem& get() noexcept;

        AudioSystem();
        AudioSystem& operator=(const AudioSystem&) = delete;

        ~AudioSystem();

        void clear();
        void prepare();

        inline bool isEnabled(bool enabled) const {
            return m_enabled;
        }

        inline void setEnabled(bool enabled) {
            m_enabled = enabled;
        }

        void prepareSource(audio::Source& source);

        audio::sound_id registerSound(std::string_view fullPath);

    private:
        bool m_prepared{ false };
        bool m_enabled{ false };

        ALCdevice* m_device{ nullptr };
        ALCcontext* m_context{ nullptr };

        std::unique_ptr<SoundRegistry> m_soundRegistry;
    };
}
