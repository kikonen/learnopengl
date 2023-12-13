#include "al_call.h"

#include <fmt/format.h>

#include "util/Log.h"

namespace audio {
    bool check_al_errors(
        const char* filename,
        int lineNumber)
    {
        ALCenum error = alGetError();
        if (error != ALC_NO_ERROR)
        {
            KI_ERROR(fmt::format("***ERROR*** ({}:{})", filename, lineNumber));

            switch (error)
            {
            case ALC_INVALID_VALUE:
                KI_ERROR(fmt::format("ALC_INVALID_VALUE: an invalid value was passed to an OpenAL function"));
                break;
            case ALC_INVALID_DEVICE:
                KI_ERROR(fmt::format("ALC_INVALID_DEVICE: a bad device was passed to an OpenAL function"));
                break;
            case ALC_INVALID_CONTEXT:
                KI_ERROR(fmt::format("ALC_INVALID_CONTEXT: a bad context was passed to an OpenAL function"));
                break;
            case ALC_INVALID_ENUM:
                KI_ERROR(fmt::format("ALC_INVALID_ENUM: an unknown enum value was passed to an OpenAL function"));
                break;
            case ALC_OUT_OF_MEMORY:
                KI_ERROR(fmt::format("ALC_OUT_OF_MEMORY: an unknown enum value was passed to an OpenAL function"));
                break;
            default:
                KI_ERROR(fmt::format("UNKNOWN ALC ERROR: {}", error));
            }
            return false;
        }
        return true;
    }
}
