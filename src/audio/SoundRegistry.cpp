#include "SoundRegistry.h"

#include "Sound.h"

namespace audio
{
    SoundRegistry::SoundRegistry()
    {
        // NOTE KI null entries to avoid need for "- 1" math
        m_sounds.emplace_back<Sound>({});
    }

    SoundRegistry::~SoundRegistry() = default;

    void SoundRegistry::clear()
    {
        std::unique_lock lock(m_lock);
        m_sounds.clear();
    }

    Sound* SoundRegistry::getSound(
        audio::sound_id id)
    {
        std::shared_lock lock(m_lock);

        if (id < 1 || id >= m_sounds.size()) return nullptr;
        return &m_sounds[id];
    }

    audio::sound_id SoundRegistry::registerSound(std::string_view fullPath)
    {
        std::unique_lock lock(m_lock);

        const auto sid = SID(fullPath);

        const auto& it = m_sidToId.find(sid);
        if (it != m_sidToId.end()) return it->second;

        audio::Sound& sound = m_sounds.emplace_back();
        sound.m_id = static_cast<audio::sound_id>(m_sounds.size() - 1);

        if (!sound.load(std::string{ fullPath })) {
            m_sounds.pop_back();
            return 0;
        }

        m_sidToId.insert({ sid, sound.m_id});

        return sound.m_id;
    }
}
