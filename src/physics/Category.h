#pragma once

#include <stdint.h>

namespace physics {
    enum class Category {
        none = 0,
        invalid = 1 << 1,
        // surroundings
        ground = 1 << 2,
        scenery = 1 << 3,
        water = 1 << 4,
        // characters and creatures
        npc = 1 << 5,
        player = 1 << 6,
        // Ray casting
        ray_player_fire = 1 << 7,
        ray_npc_fire = 1 << 8,
        ray_hit = 1 << 9,
    };
}
