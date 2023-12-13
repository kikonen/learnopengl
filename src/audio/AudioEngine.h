#pragma once

#include <map>
#include <memory>

#include "AL/alc.h"

#include "asset/Assets.h"

#include "size.h"

class UpdateContext;

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
        bool isPlaying(audio::source_id);

        void setActiveListener(audio::listener_id id);

        audio::listener_id registerListener();

        Listener* getListener(
            audio::listener_id id);

        audio::source_id registerSource(
            audio::sound_id soundId);

        Source* getSource(
            audio::source_id id);

        audio::sound_id registerSound(std::string_view fullPath);

    private:
        const Assets& m_assets;

        bool m_prepared{ false };
        bool m_enabled{ false };

        ALCdevice* m_device{ nullptr };
        ALCcontext* m_context{ nullptr };

        audio::listener_id m_activeListenerId{ 0 };

        std::map<audio::listener_id, std::unique_ptr<Listener>> m_listeners;
        std::map<audio::source_id, std::unique_ptr<Source>> m_sources;

        std::unique_ptr<SoundRegistry> m_soundRegistry;
    };
}
