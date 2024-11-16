#pragma once

#include <string>
#include <unordered_map>
#include <shared_mutex>

#include "size.h"

namespace audio
{
    struct Sound;

    class SoundRegistry final
    {
    public:
        SoundRegistry();
        ~SoundRegistry();

        void clear();

        // main thread
        Sound* getSound(audio::sound_id id);

        // worker thread
        audio::sound_id registerSound(std::string_view fullPath);

    private:
        std::unordered_map<ki::StringID, audio::sound_id> m_sidToId;
        std::vector<Sound> m_sounds;

        std::shared_mutex m_lock;
    };
}
