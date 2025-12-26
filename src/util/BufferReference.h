#pragma once

#include <stdint.h>

namespace util
{
    struct BufferReference
    {
        uint32_t offset;
        uint32_t size;

        bool operator==(const BufferReference& o) const
        {
            return offset == o.offset && size == o.size;
        }
    };
}
