#include "Script.h"

#include <mutex>

#include "size.h"

namespace {
    script::script_id idBase{ 0 };

    std::mutex id_lock{};

    script::script_id nextID()
    {
        std::lock_guard<std::mutex> lock(id_lock);
        return ++idBase;
    }
}

namespace script
{
    Script::Script(std::string_view source)
        : m_id(nextID()),
        m_source(source)
    {}
}
