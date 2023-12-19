#include "SoundRegistry.h"

#include "Sound.h"

namespace audio
{
    SoundRegistry::SoundRegistry()
    {
        // NOTE KI null entries to avoid need for "- 1" math
        m_sounds.emplace_back<Sound>({});
    }

    void SoundRegistry::clear()
    {
        std::lock_guard<std::mutex> lock(m_lock);
        m_sounds.clear();
    }

    Sound* SoundRegistry::getSound(
        audio::sound_id id)
    {
        std::lock_guard<std::mutex> lock(m_lock);

        if (id < 1 || id >= m_sounds.size()) return nullptr;
        return &m_sounds[id];
    }

    audio::sound_id SoundRegistry::registerSound(std::string_view fullPath)
    {
        std::lock_guard<std::mutex> lock(m_lock);

        const auto& it = m_pathToId.find(std::string{ fullPath });
        if (it != m_pathToId.end()) return it->second;

        auto& sound = m_sounds.emplace_back<Sound>({});
        sound.m_id = static_cast<audio::sound_id>(m_sounds.size() - 1);

        if (!sound.load(fullPath)) {
            m_sounds.pop_back();
            return 0;
        }

        m_pathToId.insert({ std::string{ fullPath }, sound.m_id });

        return sound.m_id;
    }
}
