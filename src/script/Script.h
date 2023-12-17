#pragma once

#include <string>

#include "size.h"

namespace script
{
    struct Script
    {
        Script(std::string_view source);

        script::script_id m_id;

        std::string m_source;
    };
}
