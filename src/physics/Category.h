#pragma once

#include <stdint.h>

namespace physics {
    enum class Category : uint32_t {
        none = 0,
        all = UINT_MAX,
        invalid = 1 << 1,
        // surroundings
        ground = 1 << 3,
        water = 1 << 4,
        scenery = 1 << 5,
        prop = 1 << 6,
        // characters and creatures
        npc = 1 << 10,
        player = 1 << 11,
        // characteristic
        can_float = 1 << 20,
        // Ray casting
        ray_player_fire = 1 << 25,
        ray_npc_fire = 1 << 26,
        ray_hit = 1 << 27,
    };
}
