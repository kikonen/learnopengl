#pragma once

#include <string>
#include <mutex>
#include <unordered_map>

#include "size.h"

namespace audio
{
    struct Sound;

    class SoundRegistry final
    {
    public:
        SoundRegistry() = default;
        ~SoundRegistry() = default;

        // main thread
        Sound* getSound(audio::sound_id id);

        // worker thread
        audio::sound_id registerSound(std::string_view fullPath);

    private:
        std::unordered_map<std::string, audio::sound_id> m_pathToId;
        std::unordered_map<audio::sound_id, Sound> m_sounds;

        std::mutex m_lock{};
    };
}
