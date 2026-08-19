#include "compress.h"

#include <stdexcept>

#include <zlib.h>

namespace util
{
    std::vector<unsigned char> compress_zlib(
        const void* src,
        std::size_t src_len)
    {
        uLong bound = compressBound(static_cast<uLong>(src_len));
        std::vector<unsigned char> out(bound);

        uLong out_len = bound;
        // or Z_BEST_COMPRESSION
        int rc = compress2(out.data(), &out_len,
            static_cast<const Bytef*>(src), static_cast<uLong>(src_len),
            Z_BEST_SPEED);
        if (rc != Z_OK) throw std::runtime_error("compress failed");
        out.resize(out_len);
        return out;
    }

    // --- decompress -----------------------------------------------
    std::vector<unsigned char> decompress_zlib(
        const void* src,
        std::size_t src_len,
        std::size_t original_len)
    {
        std::vector<unsigned char> out(original_len);
        uLong out_len = static_cast<uLong>(original_len);
        int rc = uncompress(out.data(), &out_len,
            static_cast<const Bytef*>(src), static_cast<uLong>(src_len));
        if (rc != Z_OK) throw std::runtime_error("decompress failed");
        out.resize(out_len);
        return out;
    }
}
