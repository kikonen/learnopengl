#include "assets_loader_util.h"

#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <filesystem>

#include <fmt/format.h>

#include "util/Log.h"

#include "util/util.h"

namespace {
    const float DEF_ALPHA = 1.0;
}

namespace assets {
    std::string readString(const YAML::Node& node)
    {
        return node.as<std::string>();
    }

    bool readBool(const YAML::Node& node)
    {
        if (!util::isBool(node.as<std::string>())) {
            KI_WARN(fmt::format("invalid bool={}", renderNode(node)));
            return false;
        }

        return node.as<bool>();
    }

    int readInt(const YAML::Node& node)
    {
        if (!util::isInt(node.as<std::string>())) {
            KI_WARN(fmt::format("invalid int={}", renderNode(node)));
            return 0;
        }

        return node.as<int>();
    }

    float readFloat(const YAML::Node& node)
    {
        if (!util::isFloat(node.as<std::string>())) {
            KI_WARN(fmt::format("invalid float={}", renderNode(node)));
            return 0.f;
        }

        return node.as<float>();
    }

    std::vector<int> readIntVector(const YAML::Node& node, int reserve)
    {
        std::vector<int> a;
        a.reserve(reserve);

        for (const auto& e : node) {
            a.push_back(readInt(e));
        }

        return a;
    }

    std::vector<float> readFloatVector(const YAML::Node& node, int reserve)
    {
        std::vector<float> a;
        a.reserve(reserve);

        for (const auto& e : node) {
            a.push_back(readFloat(e));
        }

        return a;
    }

    glm::uvec2 readUVec2(const YAML::Node& node)
    {
        const auto& a = readIntVector(node, 2);
        return glm::uvec2{ a[0], a[1] };
    }

    glm::uvec3 readUVec3(const YAML::Node& node)
    {
        const auto& a = readIntVector(node, 3);
        return glm::uvec3{ a[0], a[1], a[2] };
    }

    glm::vec2 readVec2(const YAML::Node& node)
    {
        const auto& a = readFloatVector(node, 2);
        return glm::vec2{ a[0], a[1] };
    }

    glm::vec3 readVec3(const YAML::Node& node)
    {
        const auto& a = readFloatVector(node, 3);
        return glm::vec3{ a[0], a[1], a[2] };
    }

    glm::vec4 readVec4(const YAML::Node& node)
    {
        const auto& a = readFloatVector(node, 4);
        return glm::vec4{ a[0], a[1], a[2], a[3] };
    }

    glm::vec2 readScale2(const YAML::Node& node)
    {
        std::vector<float> a;

        if (node.IsSequence()) {
            auto a = readFloatVector(node, 2);

            while (a.size() < 2) {
                a.push_back(1.0);
            }
            return glm::vec2{ a[0], a[1] };
        }

        auto scale = readFloat(node);
        return glm::vec3{ scale };
    }

    glm::vec3 readRGB(const YAML::Node& node)
    {
        if (node.IsSequence()) {
            auto a = readFloatVector(node, 3);

            if (a.size() == 0) {
                a.push_back(0.f);
                a.push_back(0.f);
                a.push_back(0.f);
            }
            else if (a.size() == 1) {
                a.push_back(a[0]);
                a.push_back(a[0]);
            }
            else if (a.size() == 2) {
                a.push_back(a[0]);
            }

            return glm::vec3{ a[0], a[1], a[2] };
        }

        auto r = readFloat(node);
        return glm::vec3{ r, r, r };
    }

    glm::vec4 readRGBA(const YAML::Node& node)
    {
        if (node.IsSequence()) {
            auto a = readFloatVector(node, 4);

            if (a.size() == 0) {
                a.push_back(0.f);
                a.push_back(0.f);
                a.push_back(0.f);
            }
            else if (a.size() == 1) {
                a.push_back(a[0]);
                a.push_back(a[0]);
            }
            else if (a.size() == 2) {
                a.push_back(a[0]);
            }

            // NOTE KI check if alpha is missing
            if (a.size() < 4) {
                a.push_back(DEF_ALPHA);
            }

            return glm::vec4{ a[0], a[1], a[2], a[3] };
        }

        auto r = readFloat(node);
        return glm::vec4{ r, r, r, DEF_ALPHA };
    }

    void reportUnknown(
        std::string_view scope,
        std::string_view k,
        const YAML::Node& v)
    {
        std::string prefix = k.starts_with("xx") ? "DISABLED" : "UNKNOWN";
        KI_WARN_OUT(fmt::format("ASSETS::{} {}: {}={}", prefix, scope, k, renderNode(v)));
    }

    std::string renderNode(
        const YAML::Node& v)
    {
        std::stringstream ss;
        ss << v;
        return ss.str();
    }
}
