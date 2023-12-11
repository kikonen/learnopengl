#pragma once

#include <map>
#include <memory>

#include <glm/glm.hpp>

#include "asset/Assets.h"

#include "size.h"
#include "Audio.h"
#include "Source.h"
#include "Listener.h"

class UpdateContext;

namespace audio
{
    class AudioEngine {
    public:
        AudioEngine(const Assets& assets);
        ~AudioEngine();

        void prepare();
        void update(const UpdateContext& ctx);

        inline bool isEnabled(bool enabled) const {
            return m_enabled;
        }

        inline void setEnabled(bool enabled) {
            m_enabled = enabled;
        }

        void playSource(audio::source_id);
        void stopSource(audio::source_id);

        void setActiveListener(audio::listener_id listenerId);

        audio::listener_id registerListener();

        Listener* updateListener(
            audio::listener_id listenerId);

        audio::source_id registerSource(
            audio::audio_id audioId);

        Source* updateSource(
            audio::source_id sourceId);

        audio::audio_id registerAudio(std::string_view path);

    private:
        const Assets& m_assets;

        bool m_prepared{ false };
        bool m_enabled{ false };

        audio::listener_id m_activeListenerId;

        std::map<audio::listener_id, std::unique_ptr<Listener>> m_listeners;
        std::map<audio::source_id, std::unique_ptr<Source>> m_sources;
        std::map<audio::audio_id, std::unique_ptr<Audio>> m_audios;
    };
}
