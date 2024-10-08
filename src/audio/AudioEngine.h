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
        void update(const UpdateContext& ctx);

        inline bool isEnabled(bool enabled) const {
            return m_enabled;
        }

        inline void setEnabled(bool enabled) {
            m_enabled = enabled;
        }

        void playSource(audio::source_id id);
        void stopSource(audio::source_id id);
        void pauseSource(audio::source_id id);
        void toggleSource(audio::source_id id, bool play);


        bool isPlaying(audio::source_id);
        bool isPaused(audio::source_id);

        void setActiveListener(audio::listener_id id);

        audio::listener_id registerListener();

        Listener* getListener(
            audio::listener_id id);

        void setListenerPos(
            audio::source_id id,
            const glm::vec3& pos,
            const glm::vec3& front,
            const glm::vec3& up);

        audio::source_id registerSource(
            audio::sound_id soundId);

        Source* getSource(
            audio::source_id id);

        void setSourcePos(
            audio::source_id id,
            const glm::vec3& pos,
            const glm::vec3& front);

        audio::sound_id registerSound(std::string_view fullPath);

    private:
        void preparePendingListeners(const UpdateContext& ctx);
        void preparePendingSources(const UpdateContext& ctx);

    private:
        bool m_prepared{ false };
        bool m_enabled{ false };

        ALCdevice* m_device{ nullptr };
        ALCcontext* m_context{ nullptr };

        audio::listener_id m_activeListenerId{ 0 };

        std::vector<Listener>  m_listeners;
        std::vector<Source> m_sources;

        std::vector<audio::listener_id> m_pendingListeners;
        std::vector<audio::source_id> m_pendingSources;

        std::unique_ptr<SoundRegistry> m_soundRegistry;
    };
}
