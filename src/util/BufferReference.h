#pragma once

#include <stdint.h>
#include <functional>

namespace util
{
    struct BufferReference
    {
        uint32_t offset{ 0 };
        uint32_t size{ 0 };

        BufferReference() = default;

        BufferReference(size_t o_offset, size_t o_size)
            : offset{ static_cast<uint32_t>(o_offset) },
            size{ static_cast<uint32_t>(o_size) }
        {
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

        bool operator==(const BufferReference& o) const = default;

        bool contains(const BufferReference o) const
        {
            return begin() <= o.begin() && o.end() <= end();
        }
    };
}

namespace std
{
    template<>
    struct hash<util::BufferReference>
    {
        size_t operator()(const util::BufferReference ref) const noexcept
        {
            size_t h1 = std::hash<uint32_t>{}(ref.offset);
            size_t h2 = std::hash<uint32_t>{}(ref.size);
            return h1 ^ (h2 << 1);
        }
    };
}
