#pragma once

#include <string>
#include <stdint.h>


namespace pool {
    struct IdHash {
        static uint32_t make32(std::string_view id_name) noexcept;
    };
}
