#pragma once

#include <vector>

namespace util
{
    std::vector<unsigned char> compress_zlib(
        const void* src,
        std::size_t src_len);
}
