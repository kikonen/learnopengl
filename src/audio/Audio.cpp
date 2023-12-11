#include "Audio.h"

#include <mutex>

#include <AudioFile.h>

#include "util/Util.h"

namespace {
    audio::audio_id idBase{ 0 };

    std::mutex id_lock{};

    audio::audio_id nextID()
    {
        std::lock_guard<std::mutex> lock(id_lock);
        return ++idBase;
    }
}

namespace audio
{
    Audio::Audio(std::string_view path)
        : m_id(nextID()),
        m_path(path)
    {
    }

    Audio::~Audio()
    {
    }

    void Audio::load(const std::string_view assetsPath)
    {
        std::string audioPath = util::joinPath(assetsPath, m_path);

        AudioFile<double> audioFile;
        audioFile.load(audioPath);

        {
            m_sampleRate = audioFile.getSampleRate();
            m_bitDepth = audioFile.getBitDepth();

            m_sampleCount = audioFile.getNumSamplesPerChannel();
            m_lengthInSeconds = audioFile.getLengthInSeconds();

            m_channelCount = audioFile.getNumChannels();
            m_isMono = audioFile.isMono();
            m_isStereo = audioFile.isStereo();

            audioFile.printSummary();
        }
    }
}
