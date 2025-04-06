#pragma once

#include <string>

#include "size.h"

#include "ScriptType.h"

namespace script
{
    struct ScriptFile
    {
        static script::script_id nextId();

        bool m_embedded{ false };
        ScriptType m_type{ ScriptType::none };
        std::string m_filePath;
        std::string m_source;

        script::script_id m_id{ 0 };
    };
}
