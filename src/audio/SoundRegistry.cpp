#include "SoundRegistry.h"

#include "Sound.h"

namespace audio
{
    Sound* SoundRegistry::getSound(
        audio::sound_id id)
    {
        std::lock_guard<std::mutex> lock(m_lock);

        const auto& it = m_sounds.find(id);
        if (it == m_sounds.end()) return nullptr;

        auto& sound = it->second;
        sound.prepare();

        return &sound;
    }

    audio::sound_id SoundRegistry::registerSound(std::string_view fullPath)
    {
        std::lock_guard<std::mutex> lock(m_lock);

        const auto& it = m_pathToId.find(std::string{ fullPath });
        if (it != m_pathToId.end()) return it->second;

        Sound sound{ fullPath };

        if (!sound.load()) return 0;

        m_sounds.insert({ sound.m_id, sound });
        m_pathToId.insert({ std::string{ fullPath }, sound.m_id });

        return sound.m_id;
    }
}
