#pragma once

#include <string>
#include <unordered_map>
#include <shared_mutex>

#include "util/Ref.h"

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
        audio::sound_id registerSound(const std::string& fullPath);

    private:
        std::unordered_map<audio::sound_id, util::Ref<audio::Sound>> m_idToSound;

        std::shared_mutex m_lock;
    };
}
