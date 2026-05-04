#pragma once

#include <stdint.h>

namespace ki
{
    typedef uint32_t sid_t;

    typedef sid_t node_id;
    typedef uint16_t mesh_id;
    typedef sid_t lod_mesh_id;
    typedef sid_t lod_group_id;
    typedef sid_t type_id;
    typedef sid_t composite_id;
    typedef sid_t tag_id;

    typedef sid_t socket_id;

    typedef sid_t material_id;
    typedef sid_t material_updater_id;
    typedef int32_t material_index;

    typedef sid_t decal_id;

    typedef uint8_t program_id;
    typedef uint8_t vao_id;

    typedef uint16_t size_t_entity_flags;

    typedef uint16_t level_id;
}
