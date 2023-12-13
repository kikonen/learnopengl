#pragma once

#include <glm/glm.hpp>

#include <stduuid/uuid.h>


namespace loader {
    struct RootData {
        uuids::uuid rootId;

        glm::vec3 pos{ 0.f };
    };
}
