#pragma once

#include "animation/RigSocket.h"

#include "loader/document.h"
#include "loader/loader_util.h"

namespace loader {
    struct SocketRoleValue {
        static animation::SocketRole load(const loader::DocNode& node)
        {
            std::string str = readString(node);
            if (str == "foot_left") return animation::SocketRole::foot_left;
            if (str == "foot_right") return animation::SocketRole::foot_right;
            if (str == "ground_sensor" || str == "ground") return animation::SocketRole::ground_sensor;
            if (str == "physics_center" || str == "capsule_center") return animation::SocketRole::physics_center;
            return animation::SocketRole::general;
        }
    };
}
