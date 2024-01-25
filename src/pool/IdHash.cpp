#include "IdHash.h"

#include "crc32c/crc32c.h"

namespace pool {
    uint32_t IdHash::make32(std::string_view id_name) noexcept
    {
        uint32_t result;
        std::string s{ id_name };
        result = crc32c::Crc32c(s);
        return result;
    }
}
