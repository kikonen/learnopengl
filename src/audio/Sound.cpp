#include "Sound.h"

#include <mutex>

#include <fmt/format.h>
#include <AudioFile.h>

#include "util/log.h"
#include "util/Util.h"

#include "al_call.h"


namespace {
    audio::sound_id idBase{ 0 };

    std::mutex id_lock{};

    audio::sound_id nextID()
    {
        std::lock_guard<std::mutex> lock(id_lock);
        return ++idBase;
    }
}

namespace audio
{
    Sound::Sound(std::string_view path)
        : m_id(nextID()),
        m_path(path)
    {
    }

    Sound::~Sound()
    {
        alDeleteBuffers(1, &m_bufferId);
    }

    void Sound::load(const std::string_view assetsPath)
    {
        std::string soundPath = util::joinPath(assetsPath, m_path);

        AudioFile<float> audioFile;
        audioFile.load(soundPath);

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

        ALenum format;
        if (m_channelCount == 1 && m_bitDepth == 8)
            //format = AL_FORMAT_MONO8;
            format = AL_FORMAT_MONO16;
        else if (m_channelCount  == 1 && m_bitDepth == 16)
            format = AL_FORMAT_MONO16;
        else if (m_channelCount == 2 && m_bitDepth == 8)
            //format = AL_FORMAT_STEREO8;
            format = AL_FORMAT_STEREO16;
        else if (m_channelCount == 2 && m_bitDepth == 16)
            format = AL_FORMAT_STEREO16;
        else
        {
            KI_CRITICAL(
                fmt::format("ERROR: unrecognised wave format: channels={}, bps={}",
                m_channelCount, m_bitDepth));
            return;
        }

        // NOTE KI convert to AL compatible format
        // http://forum.lwjgl.org/index.php?topic=4058.0
        // Stereo data is expressed in interleaved format,
        std::vector<ALint> buffer;
        {
            buffer.reserve(m_sampleCount * m_channelCount);

            // NOTE KI interleaved
            for (size_t i = 0; i < m_sampleCount; i++) {
                for (size_t channel = 0; channel < m_channelCount; channel++) {
                    const auto v = audioFile.samples[channel][i];
                    const auto converted = (ALint)(v * 65536.f / 2.f);
                    buffer.push_back(converted);
                }
            }
        }

        alGenBuffers(1, &m_bufferId);
        alBufferData(
            m_bufferId,
            format,
            (ALvoid*)buffer.data(),
            static_cast<ALsizei>(buffer.size() * sizeof(ALint)),
            m_sampleRate);
    }
}
