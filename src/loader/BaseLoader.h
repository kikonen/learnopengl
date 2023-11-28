#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "ki/uuid.h"
#include "asset/Assets.h"

#include "Context.h"
#include "BaseUUID.h"

namespace YAML {
    class Node;
}

namespace event {
    class Dispatcher;
}

class Registry;

namespace loader
{
    static const float DEF_ALPHA = 1.0;

    static const std::string AUTO_UUID{ "AUTO" };
    static const std::string ROOT_UUID{ "ROOT" };
    static const std::string VOLUME_UUID{ "VOLUME" };
    static const std::string CUBE_MAP_UUID{ "CUBE_MAP" };

    static const std::string MACRO_STEP_X{ "X" };
    static const std::string MACRO_STEP_Y{ "Y" };
    static const std::string MACRO_STEP_Z{ "Z" };

    struct Repeat {
        int xCount{ 1 };
        int yCount{ 1 };
        int zCount{ 1 };

        double xStep{ 0.f };
        double yStep{ 0.f };
        double zStep{ 0.f };
    };

    struct Tiling {
        int tile_size{ 100 };
        glm::uvec3 tiles{ 1 };
        float height_scale{ 32 };
        float horizontal_scale{ 1 };
        glm::vec2 vertical_range{ 0, 32 };
    };

    class BaseLoader
    {
    public:
        BaseLoader(
            Context ctx);

        void loadRepeat(
            const YAML::Node& node,
            Repeat& data) const;

        void loadTiling(
            const YAML::Node& node,
            Tiling& data) const;

        std::string readString(const YAML::Node& node) const;
        bool readBool(const YAML::Node& node) const;
        int readInt(const YAML::Node& node) const;
        float readFloat(const YAML::Node& node) const;

        std::vector<int> readIntVector(const YAML::Node& node, int reserve) const;
        std::vector<float> readFloatVector(const YAML::Node& node, int reserve) const;

        glm::vec2 readVec2(const YAML::Node& node) const;
        glm::vec3 readVec3(const YAML::Node& node) const;
        glm::vec4 readVec4(const YAML::Node& node) const;

        glm::uvec3 readUVec3(const YAML::Node& node) const;

        glm::vec3 readScale3(const YAML::Node& node) const;
        glm::vec3 readRGB(const YAML::Node& node) const;
        glm::vec4 readRGBA(const YAML::Node& node) const;
        glm::vec2 readRefractionRatio(const YAML::Node& node) const;

        uuids::uuid resolveUUID(
            const BaseUUID& parts,
            const int cloneIndex,
            const glm::uvec3& tile);

        std::string expandMacros(
            const std::string& str,
            const int cloneIndex,
            const glm::uvec3& tile);

        BaseUUID readUUID(const YAML::Node& node) const;

        std::string readFile(std::string_view filename) const;

        const std::string resolveTexturePath(std::string_view line) const;

        void reportUnknown(
            std::string_view scope,
            std::string_view k,
            const YAML::Node&) const;

        std::string renderNode(
            const YAML::Node& v) const;

    public:
        const Assets& m_assets;
        Context m_ctx;

        std::shared_ptr<Registry> m_registry;
        event::Dispatcher* m_dispatcher { nullptr };
    };
}
