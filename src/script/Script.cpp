#include "Script.h"

#include "size.h"

#include "pool/IdGenerator.h"

namespace {
    IdGenerator<script::script_id> ID_GENERATOR;
}

namespace script
{
    Script::Script(std::string_view source)
        : m_id(ID_GENERATOR.nextId()),
        m_source(source)
    {}
}
