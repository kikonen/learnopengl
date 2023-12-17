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
    Sound::Sound(std::string_view fullPath)
        : m_id(nextID()),
        m_fullPath(fullPath)
    {
    }

    Sound::~Sound()
    {
        if (m_bufferId) {
            alDeleteBuffers(1, &m_bufferId);
        }
    }

    void Sound::prepare()
    {
        // empty == either fail or prepared
        if (m_data.empty()) return;

        alGenBuffers(1, &m_bufferId);
        alBufferData(
            m_bufferId,
            m_format,
            (ALvoid*)m_data.data(),
            static_cast<ALsizei>(m_data.size() * sizeof(ALint)),
            m_sampleRate);
        m_data.clear();
    }

    bool Sound::load()
    {
        AudioFile<float> audioFile;
        audioFile.load(m_fullPath);

        {
            m_sampleRate = audioFile.getSampleRate();
            m_bitDepth = audioFile.getBitDepth();

            m_sampleCount = audioFile.getNumSamplesPerChannel();
            m_lengthInSeconds = audioFile.getLengthInSeconds();

            m_channelCount = audioFile.getNumChannels();
            m_isMono = audioFile.isMono();
            m_isStereo = audioFile.isStereo();

            std::cout << "file=" << m_fullPath << '\n';
            audioFile.printSummary();
        }

        auto& format = m_format;
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
            return false;
        }

        // NOTE KI convert to AL compatible format
        // http://forum.lwjgl.org/index.php?topic=4058.0
        // Stereo data is expressed in interleaved format,
        auto& buffer = m_data;
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

        return true;
    }
}
