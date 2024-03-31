#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Context.h"
#include "BaseId.h"
#include "BaseData.h"

namespace YAML {
    class Node;
}

namespace event {
    class Dispatcher;
}

struct Material;
class Registry;

namespace loader
{
    struct MaterialData;

    static const float DEF_ALPHA = 1.0;

    static const std::string ROOT_ID{ "ROOT" };

    static const std::string MACRO_STEP_CLONE{ "c" };
    static const std::string MACRO_STEP_TILE{ "t" };
    static const std::string MACRO_STEP_X{ "x" };
    static const std::string MACRO_STEP_Y{ "y" };
    static const std::string MACRO_STEP_Z{ "z" };

    class BaseLoader
    {
    public:
        BaseLoader(
            Context ctx);

        void setRegistry(std::shared_ptr<Registry> registry);

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

        // SPEC: [w, x, y, z]
        glm::quat readQuat(const YAML::Node& node) const;

        glm::vec3 readDegreesRotation(const YAML::Node& node) const;

        float readFractional(const YAML::Node& node) const;
        glm::vec2 readRefractionRatio(const YAML::Node& node) const;
        float convertRefractionRatio(const glm::vec2 ratio) const noexcept;

        std::tuple<ki::node_id, std::string> resolveId(
            const BaseId& baseId,
            const int cloneIndex,
            const glm::uvec3& tile,
            bool automatic);

        std::string expandMacros(
            const std::string& str,
            const int cloneIndex,
            const glm::uvec3& tile,
            bool automatic);

        BaseId readId(const YAML::Node& node) const;

        std::string readFile(std::string_view filename) const;

        std::string resolveTexturePath(
            std::string_view line,
            bool useCompressed) const;

        const Material* findMaterial(
            std::string_view name,
            const std::vector<MaterialData>& materials) const;

        void reportUnknown(
            std::string_view scope,
            std::string_view k,
            const YAML::Node&) const;

        std::string renderNode(
            const YAML::Node& v) const;

    public:
        Context m_ctx;

        std::shared_ptr<Registry> m_registry;
        event::Dispatcher* m_dispatcher { nullptr };
    };
}
