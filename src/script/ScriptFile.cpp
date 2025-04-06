#include "ScriptFile.h"

#include "size.h"

#include "pool/IdGenerator.h"

namespace {
    IdGenerator<script::script_id> ID_GENERATOR;
}

namespace script
{
    script::script_id ScriptFile::nextId()
    {
        return ID_GENERATOR.nextId();
    }
}
