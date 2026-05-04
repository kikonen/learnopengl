#include "SoundRegistry.h"

#include "ki/sid.h"

#include "Sound.h"

namespace audio
{
    SoundRegistry::SoundRegistry()
    {
        clear();
    }

    SoundRegistry::~SoundRegistry() = default;

    void SoundRegistry::clear()
    {
        std::unique_lock lock(m_lock);
        m_idToSound.clear();
    }

    Sound* SoundRegistry::getSound(
        audio::sound_id id)
    {
        std::shared_lock lock(m_lock);

        const auto& it = m_idToSound.find(id);
        return it != m_idToSound.end() ? it->second.get() : nullptr;
    }

    audio::sound_id SoundRegistry::registerSound(const std::string& fullPath)
    {
        std::unique_lock lock(m_lock);

        const auto sid = SID_REGISTER(fullPath).asSid();

        const auto& it = m_idToSound.find(sid);
        if (it != m_idToSound.end()) return it->second->m_id;

        auto sound = util::Ref<audio::Sound>::create();
        sound->m_id = sid;

        if (!sound->load(fullPath)) {
            return 0;
        }

        m_idToSound.insert({ sid, sound});

        return sound->m_id;
    }
}
