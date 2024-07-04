#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "model/PivotPoint.h"

#include "converter/base.h"

#include "Context.h"
#include "BaseId.h"
#include "BaseData.h"


namespace event {
    class Dispatcher;
}

struct Material;
class Registry;

namespace loader
{
    class Loaders;
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

        ~BaseLoader();

        void setRegistry(std::shared_ptr<Registry> registry);

        void loadRepeat(
            const loader::DocNode& node,
            Repeat& data) const;

        void loadTiling(
            const loader::DocNode& node,
            Tiling& data) const;

        const std::string& readString(const loader::DocNode& node) const;
        bool readBool(const loader::DocNode& node) const;
        int readInt(const loader::DocNode& node) const;
        float readFloat(const loader::DocNode& node) const;

        std::vector<std::string> readStringVector(const loader::DocNode& node, int reserve) const;
        std::vector<int> readIntVector(const loader::DocNode& node, int reserve) const;
        std::vector<float> readFloatVector(const loader::DocNode& node, int reserve) const;

        glm::vec2 readVec2(const loader::DocNode& node) const;
        glm::vec3 readVec3(const loader::DocNode& node) const;
        glm::vec4 readVec4(const loader::DocNode& node) const;

        glm::uvec3 readUVec3(const loader::DocNode& node) const;

        glm::vec3 readScale3(const loader::DocNode& node) const;

        glm::vec3 readRGB(const loader::DocNode& node) const;
        glm::vec4 readRGBA(const loader::DocNode& node) const;

        // SPEC: [w, x, y, z]
        glm::quat readQuat(const loader::DocNode& node) const;

        glm::vec3 readDegreesRotation(const loader::DocNode& node) const;

        float readFractional(const loader::DocNode& node) const;
        glm::vec2 readRefractionRatio(const loader::DocNode& node) const;
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

        BaseId readId(const loader::DocNode& node) const;

        PivotPoint readPivotPoint(const loader::DocNode& node) const;

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
            const loader::DocNode&) const;

        std::string renderNode(
            const loader::DocNode& v) const;

    public:
        Context m_ctx;

        std::shared_ptr<Registry> m_registry;
        event::Dispatcher* m_dispatcher { nullptr };
    };
}
