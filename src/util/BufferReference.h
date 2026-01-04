#pragma once

#include <stdint.h>

namespace util
{
    struct BufferReference
    {
        uint32_t offset{ 0 };
        uint32_t size{ 0 };

        bool operator==(const BufferReference& o) const
        {
            return offset == o.offset && size == o.size;
        }
    };
}
