#pragma once

#include <stdint.h>

namespace util
{
    struct BufferReference
    {
        uint32_t offset;
        uint32_t size;

        BufferReference()
            : offset{ 0 },
            size{ 0 }
        {}

        BufferReference(BufferReference& o)
            : offset{ o.offset },
            size{ o.size }
        {}

        BufferReference(const BufferReference& o)
            : offset{ o.offset },
            size{ o.size }
        {}

        BufferReference(BufferReference&& o) noexcept
            : offset{ o.offset },
            size{ o.size }
        {}

        BufferReference(size_t o_offset, size_t o_size)
            : offset{ static_cast<uint32_t>(o_offset) },
            size{ static_cast<uint32_t>(o_size) }
        {}

        ~BufferReference() = default;

        BufferReference& operator=(BufferReference& o)
        {
            offset = o.offset;
            size = o.size;
            return *this;
        }

        BufferReference& operator=(BufferReference&& o) noexcept
        {
            offset = o.offset;
            size = o.size;
            return *this;
        }

        inline bool empty() const noexcept
        {
            return size == 0;
        }

        inline uint32_t begin() const noexcept
        {
            return offset;
        }

        inline uint32_t end() const noexcept
        {
            return offset + size;
        }

        bool operator==(const BufferReference& o) const
        {
            return offset == o.offset && size == o.size;
        }

        bool contains(const BufferReference& o) const
        {
            return begin() <= o.begin() && o.end() <= end();
        }
    };
}
