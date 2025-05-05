#pragma once

#include <string>

#include <recastnavigation/Recast.h>

#include "util/Log.h"

namespace nav
{
    class BuildContext : public rcContext
    {
    public:
        BuildContext() = default;

    protected:
        virtual void doResetLog() {}
        virtual void doLog(const rcLogCategory category, const char* msg, const int len)
        {
            std::string text{ msg, static_cast<size_t>(len) };

            switch (category) {
            case RC_LOG_ERROR:
                KI_ERROR(text);
                break;
            case RC_LOG_WARNING:
                KI_WARN_OUT(text);
                break;
            case RC_LOG_PROGRESS:
                KI_INFO_OUT(text);
                break;
            }
        }
    };
}
